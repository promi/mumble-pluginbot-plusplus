/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 Natenom
    Copyright (c) 2016 while-loop
    Copyright (c) 2016 dafoxia
    Copyright (c) 2016 Phobos (promi) <prometheus@unterderbruecke.de>
    Copyright (c) 2017 Phobos (promi) <prometheus@unterderbruecke.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "pluginbot/plugins/mpd.hh"

#include <sstream>
#include <thread>
#include <algorithm>
#include <cstdlib>

#include "mpd/client.hh"
#include "mpd/status-listener.hh"
#include "pluginbot/html.hh"

namespace MumblePluginBot
{
  struct MpdPlugin::Impl
  {
    struct CommandHelp
    {
      std::string argument;
      std::string description;
    };

    struct CommandArgs
    {
      const MumbleProto::TextMessage &msg;
      const std::string &command;
      const std::string &arguments;
      Mpd::Client &mpd_client;
    };

    struct Command
    {
      std::function<void (const CommandArgs&)> invoke;
      std::vector<CommandHelp> help;
    };

    inline Impl (const Aither::Log &log, Settings &settings, Mumble::Client &client,
                 Mumble::AudioPlayer &player, MessagesPlugin &messages)
      : m_log (log), settings (settings), client (client), player (player),
        messages (messages)
    {
      init_commands ();
    }
    const Aither::Log &m_log;
    Settings &settings;
    Mumble::Client &client;
    Mumble::AudioPlayer &player;
    MessagesPlugin &messages;
    std::string info_template;
    std::thread idle_thread;
    std::thread song_thread;
    std::unique_ptr<Mpd::StatusListener> mpd_status_listener;
    std::function<void (const std::string &)> channel_message;
    std::function<void (const std::string &)> private_message;
    std::map<std::string, Command> m_commands;
    inline void init_info_template (const std::string &controlstring)
    {
      std::stringstream ss;
      ss << "send " << b_tag (controlstring + "help");
      ss << " or " << b_tag (controlstring + "about");
      ss << " for more information about me.";
      info_template = ss.str ();
    }
    void status_update (const FlagSet<Mpd::Idle> &idle_flags);
    void status_update (const FlagSet<Mpd::Idle> &idle_flags, Mpd::Status &status);
    void idle_thread_proc ();
    void update_song (Mpd::Song &song);
    void song_thread_proc ();
    void init_commands ();
    void seek (const CommandArgs &ca);
    void crossfade (const CommandArgs &ca);
    std::string time_decode (uint time);
    std::vector<uint> split_timecode (const std::string &timecode);
  };

  MpdPlugin::MpdPlugin (const Aither::Log &log, Settings &settings,
                        Mumble::Client &cli,
                        Mumble::AudioPlayer &player, MessagesPlugin &messages)
    : Plugin (log, settings, cli, player),
      pimpl (std::make_unique<Impl> (this->m_log, this->settings (), this->client (),
                                     this->player (), messages))
  {
  }

  MpdPlugin::~MpdPlugin ()
  {
    if (pimpl->mpd_status_listener != nullptr)
      {
        pimpl->mpd_status_listener->stop ();
        pimpl->idle_thread.join ();
      }
    pimpl->song_thread.join ();
  }

  void MpdPlugin::Impl::status_update (const FlagSet<Mpd::Idle> &idle_flags)
  {
    try
      {
        Mpd::Client client {settings.mpd.host, settings.mpd.port};
        auto status = client.status ();
        status_update (idle_flags, status);
      }
    catch (std::runtime_error &e)
      {
        channel_message (red_bold_span (std::string {"An error occured: "} +
                                        e.what ()));
      }
  }

  void MpdPlugin::Impl::status_update (const FlagSet<Mpd::Idle> &idle_flags,
                                       Mpd::Status &status)
  {
    if (idle_flags.test (Mpd::Idle::Mixer))
      {
        messages.send_message ("Volume was set to " +
                               std::to_string (status.volume ()) + "%",
                               MessageType::Volume);
      }
    if (idle_flags.test (Mpd::Idle::Database))
      {
        if (settings.chan_notify.test (MessageType::UpdatingDB))
          {
            channel_message ("I am running a database update just now ... new songs :)");
            // TODO: What is a jobid, check where ruby-mpd got it if relevant.
            // "<br>My job id is: #{jobid}."
          }
      }
    if (idle_flags.test (Mpd::Idle::Options))
      {
        auto chan_notify = settings.chan_notify;
        if (chan_notify.test (MessageType::Random))
          {
            channel_message (std::string {"Random mode is now: "} +
                             (status.random () ? "On" : "Off"));
          }
        if (chan_notify.test (MessageType::State))
          {
            std::string state;
            switch (status.state ())
              {
              case Mpd::State::Unknown:
                state = "state is unknown";
                break;
              case Mpd::State::Stop:
                state = "stopped";
                break;
              case Mpd::State::Play:
                state = "playing";
                break;
              case Mpd::State::Pause:
                state = "paused";
                break;
              default:
                state = "state is unknown (not implemented)";
              }
            channel_message ("Music " + state + ".");
          }
        if (chan_notify.test (MessageType::Single))
          {
            channel_message (std::string {"Single mode is now: "} +
                             (status.single () ? "On" : "Off"));
          }
        if (chan_notify.test (MessageType::Consume))
          {
            channel_message (std::string {"Consume mode is now: "} +
                             (status.consume () ? "On" : "Off"));
          }
        if (chan_notify.test (MessageType::XFade))
          {
            auto xfade = status.crossfade ();
            if (xfade == 0)
              {
                channel_message ("Crossfade is now: Off");
              }
            else
              {
                channel_message ("Crossfade time (in seconds) is now: " +
                                 std::to_string (xfade));
              }
          }
        if (chan_notify.test (MessageType::Repeat))
          {
            channel_message (std::string {"Repeat mode is now: "} +
                             (status.repeat () ? "On" : "Off"));
          }
      }
  }

  void MpdPlugin::Impl::update_song (Mpd::Song &song)
  {
    auto artist = song.tag (Mpd::TagType::Artist, 0);
    auto title = song.tag (Mpd::TagType::Title, 0);
    auto album = song.tag (Mpd::TagType::Album, 0);
    auto file = song.uri ();
    if (settings.use_comment_for_status_display)
      {
        std::stringstream image {settings.logo};
        /*
          if ( @@bot[:youtube_downloadsubdir] != nil ) && ( @@bot[:mpd_musicfolder] != nil )
          if File.exist?(@@bot[:mpd_musicfolder]+current.file.to_s.chomp(File.extname(current.file.to_s))+".jpg")
          image = @@bot[:cli].get_imgmsg(@@bot[:mpd_musicfolder]+current.file.to_s.chomp(File.extname(current.file.to_s))+".jpg")
          else
          image = @@bot[:logo]
          end
          else
          image = @@bot[:logo]
          end
        */
        std::stringstream output {"<br><table>"};
        if (artist != nullptr)
          {
            output << "<tr><td>Artist:</td><td>" << *artist << "</td></tr>";
          }
        if (title != nullptr)
          {
            output << "<tr><td>Title:</td><td>" << *title << "</td></tr>";
          }
        if (album != nullptr)
          {
            output << "<tr><td>Album:</td><td>" << *album << "</td></tr>";
          }
        if (artist == nullptr && title == nullptr && album == nullptr)
          {
            output << "<tr><td>Source:</td><td>" << file << "</td></tr>";
          }
        output << "</table><br>" + info_template;
        client.comment (output.str ()); // image.str () + output.str ());
      }
    else
      {
        if (settings.chan_notify.test (MessageType::State))
          {
            std::string msg;
            if (artist == nullptr || title == nullptr || album == nullptr)
              {
                msg = file;
              }
            else
              {
                msg = *artist + " - " + *title + " (" + *album + ")";
              }
            channel_message (msg);
          }
      }
  }

  void MpdPlugin::Impl::song_thread_proc ()
  {
    Mpd::Client client {settings.mpd.host, settings.mpd.port};
    client.volume (settings.initial_volume);
    std::string last_file;
    bool init = true;
    while (true)
      {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for (1s);
        try
          {
            auto song = client.current_song ();
            auto file = song.uri ();
            if (init || file != last_file)
              {
                init = false;
                last_file = file;
                update_song (song);
                AITHER_DEBUG("[displayinfo] update");
              }
          }
        catch(std::runtime_error &e)
          {
            AITHER_DEBUG("Error updating song");
          }
      }
  }

  void MpdPlugin::internal_init ()
  {
    auto &settings = Plugin::settings ();
    pimpl->channel_message = [this] (const std::string &m)
    {
      channel_message (m);
    };
    pimpl->private_message = [this] (const std::string &m)
    {
      private_message (m);
    };
    pimpl->init_info_template (settings.controlstring);
    pimpl->mpd_status_listener = std::make_unique<Mpd::StatusListener>
                                 (settings.mpd.host, settings.mpd.port, [this] (auto idle_flags)
    {
      pimpl->status_update (idle_flags);
    });
    pimpl->idle_thread = std::thread([&]
    {
      pimpl->mpd_status_listener->run ();
      switch (pimpl->mpd_status_listener->state ())
        {
        case Mpd::StatusListener::State::Stopped:
          break;
        case Mpd::StatusListener::State::UnableToConnect:
          throw std::runtime_error ("Could not connect to mpd");
          break;
        case Mpd::StatusListener::State::Disconnected:
          throw std::runtime_error ("mpd connection lost");
          break;
        default:
          throw std::runtime_error ("Unknown state after running mpd status listener");
          break;
        }
    });
    player ().stream_named_pipe (settings.mpd.fifopath);
    pimpl->song_thread = std::thread([&]
    {
      pimpl->song_thread_proc ();
    });
  }

  std::string MpdPlugin::internal_name ()
  {
    return "mpd";
  }

  void MpdPlugin::Impl::init_commands ()
  {
    m_commands =
    {
      {
        "seek", {
          [this] (auto ca)
          {
            seek (ca);
          },
          {
            {"value", "Seek to an absolute position (in seconds)."},
            {"+-value", "Seek relative to current position (in seconds)."},
            {"mm:ss", "Seek to an absolute position"},
            {"+/-mm:ss", "Seek relative to current position"},
            {"hh::mm:ss", "Seek to an absolute position"},
            {"+/-hh::mm:ss", "Seek relative to current position"}
          }
        }
      }
    };
    /*
    h << f ("settings", "Print current MPD settings.");
    h << g ("crossfade", "value",
            "Set Crossfade to value seconds, 0 to disable crossfading.");
    h << f ("next", "Play next title in the queue.");
    h << f ("prev", "Play previous title in the queue.");
    h << f ("clear", "Clear the playqueue.");
    h << f ("random", "Toggle random mode.");
    h << f ("single", "Toggle single mode.");
    h << f ("repeat", "Toggle repeat mode.");
    h << f ("consume",
            "Toggle consume mode. If this mode is enabled, songs will be "
            "removed from the play queue once they were played.");
    h << f ("pp", "Toggle pause/play.");
    h << f ("stop", "Stop playing.");
    h << f ("play", "Start playing.");
    h << g ("play", "first", "Play the first song in the queue.");
    h << g ("play", "last", "Play the last song in the queue.");
    h << g ("play", "number", "Play title on position <i>number</i> in queue.");
    h << f ("songlist", "Print the list of ALL songs in the MPD collection.");
    h << g ("playlist", "id", "Load the playlist referenced by the id.");
    h << g ("saveplaylist", "name", "Save queue into a playlist named 'name'");
    h << g ("delplaylist", "id", "Remove a playlist with the given id. "
            "Use " + controlstring + "playlists to get a list of "
            "available playlists.");
    h << f ("song", "Print some information about the currently played song.");
    h << f ("status", "Print current status of MPD.");
    h << f ("playlists", "Print the available playlists from MPD.");
    h << g ("add", "searchstring",
            "Find song(s) by searchstring and print matches.");
    h << g ("delete", "ID", "Delete an entry from the current queue. "
            "Use " + controlstring + "queue to get the IDs of all songs in "
            "the current queue.");
    h << g ("where", "searchstring",
            "Find song(s) by searchstring and print matches.");
    h << f ("queue", "Print the current play queue.");
    h << f ("stats", "Print some interesing MPD statistics.");
    h << f ("shuffle", "Play songs from the play queue in a random order.");
    h << f ("file", "Print the filename of the current song. "
            "This is useful if the file doesn't have ID3 tags "
            "and so the " + controlstring + b_tag ("song") + " command shows nothing.");
    h << f ("v++++", "Turns volume 20% up.");
    h << f ("v-", "Turns volume 5% down.");
    h << g ("v", "value", "Set the volume to the given value.");
    h << f ("v", "Print the current playback volume.");
    h << f ("update", "Start a MPD database update.");
    h << f ("mpdconfig", "Try to read mpd config.");
    h << f ("mpdcommands", "Show what commands mpd do allow to Bot (not to you!).");
    h << f ("mpdnotcommands", "Show what commands mpd disallowed to Bot.");
    h << f ("mpddecoders", "Show enabled decoders and "
            "what they can decode for your mpd.");
    */
  }

  std::string MpdPlugin::internal_help ()
  {
    const std::string &controlstring = pimpl->settings.controlstring;

    std::stringstream h;
    h << hr_tag + red_span ("Plugin " + name ()) + br_tag;
    for (const auto &p : pimpl->m_commands)
      {
        for (auto help : p.second.help)
          {
            std::string argstr;
            if (help.argument != "")
              {
                argstr = " " + i_tag (help.argument);
              }
            h << b_tag (controlstring + p.first + argstr)
              + " - " + help.description + br_tag;
          }
      }
    return h.str ();
  }

  void MpdPlugin::Impl::seek (const CommandArgs &ca)
  {
    if (ca.arguments != "")
      {
        bool negative;
        bool absolute;
        std::string timecode;
        if (ca.arguments[0] == '+')
          {
            timecode = ca.arguments.substr (1);
            absolute = false;
            negative = false;
          }
        else if (ca.arguments[0] == '-')
          {
            timecode = ca.arguments.substr (1);
            absolute = false;
            negative = true;
          }
        else
          {
            timecode = ca.arguments;
            absolute = true;
            negative = false;
          }
        auto parts = split_timecode (timecode);
        uint seconds;
        switch (parts.size ())
          {
          case 1:
            seconds = parts[0];
            break;
          case 2:
            if (parts[1] > 59)
              {
                throw std::invalid_argument ("seconds out of range");
              }
            seconds = parts[0] * 60 + parts[1];
            break;
          case 3:
            if (parts[1] > 59)
              {
                throw std::invalid_argument ("minutes out of range");
              }
            if (parts[2] > 59)
              {
                throw std::invalid_argument ("seconds out of range");
              }
            seconds = parts[0] * 60 * 60 + parts[1] * 60 + parts[2];
            break;
          default:
            throw std::invalid_argument ("invalid count of `:` chars");
          }
        auto status = ca.mpd_client.status ();
        auto elapsed = status.elapsed_time ();
        auto id = status.song_id ();
        uint t;
        if (absolute)
          {
            t = seconds;
          }
        else
          {
            t = elapsed + seconds * (negative ? -1 : 1);
          }
        AITHER_DEBUG("t = " << t);
        ca.mpd_client.seek_id (id, t);
      }
    // Warning: Don't reuse the old status from before seeking.
    // The chat message has to reflect the *new* position!
    auto status = ca.mpd_client.status ();
    auto elapsed = status.elapsed_time ();
    auto total = status.total_time ();
    private_message ("Now on position " +
                     time_decode (elapsed) + "/" + time_decode (total) + ".");
  }

  void MpdPlugin::Impl::crossfade (const CommandArgs &ca)
  {
    try
      {
        ca.mpd_client.crossfade (std::stoi (ca.arguments));
      }
    catch (std::invalid_argument &e)
      {
        private_message ("Invalid argument");
      }
  }

  void MpdPlugin::internal_chat (const MumbleProto::TextMessage &msg,
                                 const std::string &command, const std::string &arguments)
  {
    auto commands = pimpl->m_commands;
    auto cmd = commands.find (command);
    if (cmd == std::end (commands))
      {
        return;
      }
    Mpd::Client mpd_client {pimpl->settings.mpd.host, pimpl->settings.mpd.port};
    Impl::CommandArgs ca {msg, command, arguments, mpd_client};
    cmd->second.invoke (ca);
    // if command == 'mpdhelp' ...
    /*
    @@bot[:mpd].next if message == 'next'
    @@bot[:mpd].previous if message == 'prev'

    if message == 'clear'
    @@bot[:mpd].clear
    privatemessage( "The playqueue was cleared.")
    end

    if message[0,6] == 'delete'
    begin
    @@bot[:mpd].delete message.split(/ /)[1]
    rescue
    privatemessage( "Sorry, could not delete.")
    end
    end

    if message == 'random'
    @@bot[:mpd].random = !@@bot[:mpd].random?
    end

    if message == 'repeat'
    @@bot[:mpd].repeat = !@@bot[:mpd].repeat?
    end

    if message == 'single'
    @@bot[:mpd].single = !@@bot[:mpd].single?
    end

    if message == 'consume'
    @@bot[:mpd].consume = !@@bot[:mpd].consume?
    end

    if message == 'pp'
    @@bot[:mpd].pause = !@@bot[:mpd].paused?
    end

    @@bot[:mpd].stop if message == 'stop'

    if message == 'play'
    @@bot[:mpd].play
    @@bot[:cli].me.deafen false if @@bot[:cli].me.deafened?
    @@bot[:cli].me.mute false if @@bot[:cli].me.muted?
    end

    if message == 'play first'
    begin
    @@bot[:mpd].play 0
    privatemessage("Playing first song in the queue (0).")
    rescue
    privatemessage("There is no title in the queue, cant play the first entry.")
    end
    end

    if message == 'play last'
    if @@bot[:mpd].queue.length > 0
    lastsongid = @@bot[:mpd].queue.length.to_i - 1
    @@bot[:mpd].play (lastsongid)
    privatemessage("Playing last song in the queue (#{lastsongid}).")
    else
    privatemessage("There is no title in the queue, cant play the first entry.")
    end
    end

    if message.match(/^play [0-9]{1,3}$/)
    tracknumber = message.match(/^play ([0-9]{1,3})$/)[1].to_i
    begin
    @@bot[:mpd].play tracknumber
    rescue
    privatemessage("Title on position #{tracknumber.to_s} does not exist")
    end
    @@bot[:cli].me.deafen false if @@bot[:cli].me.deafened?
    @@bot[:cli].me.mute false if @@bot[:cli].me.muted?
    end

    if message == 'songlist'
    block = 0
    out = ""
    @@bot[:mpd].songs.each do |song|
    if block >= 50
    privatemessage(out.to_s)
    out = ""
    block = 0
    end
    out << "<br/>" + song.file.to_s
    block += 1
    end
    privatemessage(out.to_s)
    end

    if message == 'stats'
    out = "<table>"
    @@bot[:mpd].stats.each do |key, value|
    case
    when key.to_s == 'uptime'
    out << "<tr><td>#{key}</td><td>#{timedecode(value)}</td></tr>"
    when key.to_s == 'playtime'
    out << "<tr><td>#{key}</td><td>#{timedecode(value)}</td></tr>"
    when key.to_s == 'db_playtime'
    out << "<tr><td>#{key}</td><td>#{timedecode(value)}</td></tr>"
    else
    out << "<tr><td>#{key}</td><td>#{value}</td></tr>"
    end
    end
    out << "</table>"
    privatemessage( out)
    end

    if message == 'queue'
    if @@bot[:mpd].queue.length > 0
    text_out ="<table><th><td>#</td><td>Name</td></th>"
    songnr = 0

    @@bot[:mpd].queue.each do |song|
    if song.title.to_s.empty?
    text_out << "<tr><td>#{songnr}</td><td>No ID / Stream? Source: #{song.file}</td></tr>"
    else
    text_out << "<tr><td>#{songnr}</td><td>#{song.title}</td></tr>"
    end
    songnr += 1
    end
    text_out << "</table>"
    else
    text_out = "The queue is empty."
    end

    privatemessage( text_out)
    end

    if message[0,12] == 'saveplaylist'
    name = message.gsub("saveplaylist", "").lstrip
    if name != ""
    puts name
    playlist = MPD::Playlist.new(@@bot[:mpd], name)
    @@bot[:mpd].queue.each do |song|
    playlist.add song
    end

    privatemessage( "The playlist \"#{name}\" was created.
    Use the command #{@@bot[:controlstring]}playlists to get a list of all available playlists." )
    else
    privatemessage( "no playlist name gaven.")
    end
    end

    if message.match(/^delplaylist [0-9]{1,3}.*$/)
    playlist_id = message.match(/^delplaylist ([0-9]{1,3})$/)[1].to_i
    begin
    playlist = @@bot[:mpd].playlists[playlist_id]
    playlist.destroy
    privatemessage( "The playlist \"#{playlist.name}\" deleted.")
    rescue
    privatemessage( "Sorry, the given playlist id does not exist.")
    end
    end

    if ( message[0,5] == 'where' )
    search = message.gsub("where", "").lstrip.tr('"\\','')
    text_out = "you should search not nothing!"
    if search != ""
    text_out ="found:<br/>"
    @@bot[:mpd].where(any: "#{search}").each do |song|
    text_out << "#{song.file}<br/>"
    end
    end
    privatemessage( text_out)
    end

    if ( message[0,3] == 'add' )
    search = (message.gsub("add", "").lstrip).tr('"\\','')
    text_out = "search is empty"
    if search != ""
    text_out ="added:<br/>"
    count = 0
    @@bot[:mpd].where(any: "#{search}").each do |song|
    text_out << "add #{song.file}<br/>"
    @@bot[:mpd].add(song)
    count += 1
    end
    text_out = "found nothing" if count == 0
    end
    privatemessage( text_out)
    end

    if message == 'playlists'
    text_out = ""
    counter = 0
    @@bot[:mpd].playlists.each do |pl|
    text_out = text_out + "#{counter} - #{pl.name}<br/>"
    counter += 1
    end
    privatemessage( "I know the following playlists:<br>#{text_out}")
    end

    if message.match(/^playlist [0-9]{1,3}.*$/)
    playlist_id = message.match(/^playlist ([0-9]{1,3})$/)[1].to_i
    begin
    playlist = @@bot[:mpd].playlists[playlist_id]
    @@bot[:mpd].clear
    playlist.load
    @@bot[:mpd].play
    privatemessage( "The playlist \"#{playlist.name}\" was loaded and starts now.")
    rescue
    privatemessage( "Sorry, the given playlist id does not exist.")
    end
    end

    if message == 'status'
    out = "<table>"
    @@bot[:mpd].status.each do |key, value|

    case
    when key.to_s == 'volume'
    out << "<tr><td>Current volume:</td><td>#{value}%</td></tr>"
    when key.to_s == 'repeat'
    if value
    repeat = "on"
    else
    repeat = "off"
    end
    out << "<tr><td>Repeat mode:</td><td>#{repeat}</td></tr>"
    when key.to_s == 'random'
    if value
    random = "on"
    else
    random = "off"
    end
    out << "<tr><td>Random mode:</td><td>#{random}</td></tr>"
    when key.to_s == 'single'
    if value
    single = "on"
    else
    single = "off"
    end
    out << "<tr><td>Single mode:</td><td>#{single}</td></tr>"
    when key.to_s == 'consume'
    if value
    consume = "on"
    else
    consume = "off"
    end
    out << "<tr><td>Consume mode:</td><td>#{consume}</td></tr>"
    when key.to_s == 'playlist'
    out << "<tr><td>Current playlist:</td><td>#{value}</td></tr>"

    #FIXME Not possible, because the "value" in this context is random(?) after every playlist loading.
    #playlist = @@bot[:mpd].playlists[value.to_i]
    #if not playlist.nil?
    #  out << "<tr><td>Current playlist:</td><td>#{playlist.name}</td></tr>"
    #else
    #  out << "<tr><td>Current playlist:</td><td>#{value}</td></tr>"
    #end
    when key.to_s == 'playlistlength'
    out << "<tr><td>Song count in current queue/playlist:</td><td valign='bottom'>#{timedecode(value)}</td></tr>"
    when key.to_s == 'mixrampdb'
    out << "<tr><td>Mixramp db:</td><td>#{value}</td></tr>"
    when key.to_s == 'state'
    case
    when value.to_s == 'play'
    state = "playing"
    when value.to_s == 'stop'
    state = "stopped"
    when value.to_s == 'pause'
    state = "paused"
    else
    state = "unknown state"
    end
    out << "<tr><td>Current state:</td><td>#{state}</td></tr>"
    when key.to_s == 'song'
    current = @@bot[:mpd].current_song
    if not current.nil?
    out << "<tr><td>Current song:</td><td>#{current.artist} - #{current.title} (#{current.album})</td></tr>"
    else
    out << "<tr><td>Current song:</td><td>#{value})</td></tr>"
    end
    when key.to_s == 'songid'
    #queue = Queue.new
    ##queue = @@bot[:mpd].queue
    #puts "queue: " + queue.inspect
    #current_song = queue.song_with_id(value.to_i)

    #out << "<tr><td>Current songid:</td><td>#{current_song}</td></tr>"
    out << "<tr><td>Current songid:</td><td>#{value}</td></tr>"
    when key.to_s == 'time'
    out << "<tr><td>Current position:</td><td>#{timedecode(value[0])}/#{timedecode(value[1])}</td></tr>"
    when key.to_s == 'elapsed'
    out << "<tr><td>Elapsed:</td><td>#{timedecode(value)}</td></tr>"
    when key.to_s == 'bitrate'
    out << "<tr><td>Current song bitrate:</td><td>#{value}</td></tr>"
    when key.to_s == 'audio'
    out << "<tr><td>Audio properties:</td><td>samplerate(#{value[0]}), bitrate(#{value[1]}), channels(#{value[2]})</td></tr>"
    when key.to_s == 'nextsong'
    out << "<tr><td>Position ID of next song to play (in the queue):</td><td valign='bottom'>#{value}</td></tr>"
    when key.to_s == 'nextsongid'
    out << "<tr><td>Song ID of next song to play:</td><td valign='bottom'>#{value}</td></tr>"
    else
    out << "<tr><td>#{key}:</td><td>#{value}</td></tr>"
    end

    end
    out << "</table>"
    privatemessage(out)
    end

    if message == 'file'
    current = @@bot[:mpd].current_song
    privatemessage( "Filename of currently played song:<br>#{current.file}</span>") if not current.nil?
    end

    if message == 'song'
    current = @@bot[:mpd].current_song
    if not current.nil? #Would crash if playlist was empty.
    privatemessage( "#{current.artist} - #{current.title} (#{current.album})")
    else
    privatemessage( "No song is played currently.")
    end
    end

    if message == 'shuffle'
    @@bot[:mpd].shuffle
    privatemessage( "Shuffle, shuffle and get a new order. :)")
    end

    if message == 'v'
    volume = @@bot[:mpd].volume
    privatemessage( "Current volume is #{volume}%.")
    end

    if message.match(/^v [0-9]{1,3}$/)
    volume = message.match(/^v ([0-9]{1,3})$/)[1].to_i

    if (volume >=0 ) && (volume <= 100)
    @@bot[:mpd].volume = volume
    else
    privatemessage( "Volume can be within a range of 0 to 100")
    end
    end

    if message.match(/^v[-]+$/)
    multi = message.match(/^v([-]+)$/)[1].scan(/\-/).length
    volume = ((@@bot[:mpd].volume).to_i - 5 * multi)
    if volume < 0
    channelmessage( "Volume can't be set to &lt; 0.")
    volume = 0
    end
    @@bot[:mpd].volume = volume
    end

    if message.match(/^v[+]+$/)
    multi = message.match(/^v([+]+)$/)[1].scan(/\+/).length
    volume = ((@@bot[:mpd].volume).to_i + 5 * multi)
    if volume > 100
    channelmessage( "Volume can't be set to &gt; 100.")
    volume = 100
    end
    @@bot[:mpd].volume = volume
    end

    if message == 'update'
    @@bot[:mpd].update
    privatemessage("Running database update...")
    while @@bot[:mpd].status[:updating_db] != nil do
    sleep 0.5
    end
    privatemessage("Done.")
    end

    if message == 'displayinfo'
    begin
    if @@bot[:use_comment_for_status_display] == true
    @@bot[:use_comment_for_status_display] = false
    privatemessage( "Output is now \"Channel\"")
    @@bot[:cli].set_comment(@template_if_comment_disabled % [@controlstring])
    else
    @@bot[:use_comment_for_status_display] = true
    privatemessage( "Output is now \"Comment\"")
    @@bot[:cli].set_comment(@template_if_comment_enabled)
    end
    rescue NoMethodError
    if @@bot[:debug]
    puts "#{$!}"
    end
    end
    end

    if message == 'mpdconfig'
    begin
    config = @@bot[:mpd].config
    rescue
    config = "Configuration only for local clients readable"
    end
    privatemessage( config)
    end

    if message == 'mpdcommands'
    output = ""
    @@bot[:mpd].commands.each do |command|
    output << "<br>#{command}"
    end
    privatemessage( output)
    end

    if message == 'mpdnotcommands'
    output = ""
    @@bot[:mpd].notcommands.each do |command|
    output << "<br\>#{command}"
    end
    privatemessage( output)
    end

    if message == 'mpdurlhandlers'
    output = ""
    @@bot[:mpd].url_handlers.each do |handler|
    output << "<br\>#{handler}"
    end
    privatemessage( output)
    end

    if message == 'mpddecoders'
    output = "<table>"
    @@bot[:mpd].decoders.each do |decoder|
    output << "<tr>"
    output << "<td>#{decoder[:plugin]}</td>"
    output << "<td>"
    begin
    decoder[:suffix].each do |suffix|
    output << "#{suffix} "
    end
    output << "</td>"
    rescue
    output << "#{decoder[:suffix]}"
    end
    end
    output << "</table>"
    privatemessage( output)
    end
    end
    */
  }

  std::string MpdPlugin::Impl::time_decode (uint time)
  {
    // Code from https://stackoverflow.com/questions/19595840/rails-get-the-time-difference-in-hours-minutes-and-seconds
    auto mm_ss = std::div (time, 60);
    auto hh_mm = std::div (mm_ss.quot, 60);
    auto dd_hh = std::div (hh_mm.quot, 24);
    const size_t max_size = 30;
    char data[max_size] = {0};
    size_t size;
    if (mm_ss.quot < 60)
      {
        size = snprintf (data, max_size, "%02d:%02d",
                         mm_ss.quot, mm_ss.rem);
      }
    else if (hh_mm.quot < 24)
      {
        size = snprintf (data, max_size, "%02d:%02d:%02d",
                         hh_mm.quot, hh_mm.rem, mm_ss.rem);
      }
    else
      {
        size = snprintf (data, max_size, "%04d days %02d:%02d:%02d",
                         dd_hh.quot, dd_hh.rem, hh_mm.rem, mm_ss.rem);
      }
    return std::string {data, data + size};
  }

  std::vector<uint> MpdPlugin::Impl::split_timecode (const std::string &timecode)
  {
    std::stringstream ss {timecode};
    std::vector<uint> v;
    for (std::string part; std::getline (ss, part, ':'); )
      {
        v.push_back (std::stoi (part));
      }
    return v;
  }
}
