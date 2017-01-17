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

#include "mpd/client.hh"
#include "mpd/status-listener.hh"
#include "pluginbot/html.hh"

namespace MumblePluginBot
{
  struct MpdPlugin::Impl
  {
    inline Impl (MessagesPlugin &messages) : messages (messages)
    {
    }
    MessagesPlugin &messages;
    Mumble::Client *client;
    Settings *settings;
    std::string info_template;
    std::thread idle_thread;
    std::unique_ptr<Mpd::StatusListener> mpd_status_listener;
    std::function<void (const std::string &)> channel_message;
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
  };

  MpdPlugin::MpdPlugin (MessagesPlugin &messages)
  {
    pimpl = std::make_unique<Impl> (messages);
  }

  MpdPlugin::~MpdPlugin ()
  {
    if (pimpl->mpd_status_listener != nullptr)
      {
        pimpl->mpd_status_listener->stop ();
        pimpl->idle_thread.join ();
      }
  }

  void MpdPlugin::Impl::status_update (const FlagSet<Mpd::Idle> &idle_flags)
  {
    try
      {
        Mpd::Client client {settings->mpd.host, settings->mpd.port};
        auto status = client.run_status ();
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
        if (settings->chan_notify.test (MessageType::UpdatingDB))
          {
            channel_message ("I am running a database update just now ... new songs :)");
            // TODO: What is a jobid, check where ruby-mpd got it if relevant.
            // "<br>My job id is: #{jobid}."
          }
      }
    if (idle_flags.test (Mpd::Idle::Options))
      {
        if (settings->chan_notify.test (MessageType::Random))
          {
            channel_message (std::string {"Random mode is now: "} +
                             (status.random () ? "On" : "Off"));
          }
        if (settings->chan_notify.test (MessageType::State))
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
        if (settings->chan_notify.test (MessageType::Single))
          {
            channel_message (std::string {"Single mode is now: "} +
                             (status.single () ? "On" : "Off"));
          }
        if (settings->chan_notify.test (MessageType::Consume))
          {
            channel_message (std::string {"Consume mode is now: "} +
                             (status.consume () ? "On" : "Off"));
          }
        if (settings->chan_notify.test (MessageType::XFade))
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
        if (settings->chan_notify.test (MessageType::Repeat))
          {
            channel_message (std::string {"Repeat mode is now: "} +
                             (status.repeat () ? "On" : "Off"));
          }
      }
  }

  void MpdPlugin::Impl::update_song (Mpd::Song &song)
  {
    std::unique_ptr<std::string> artist {song.tag (Mpd::TagType::Artist, 0)};
    std::unique_ptr<std::string> title {song.tag (Mpd::TagType::Title, 0)};
    std::unique_ptr<std::string> album {song.tag (Mpd::TagType::Album, 0)};
    std::string file {song.uri ()};
    if (settings->use_comment_for_status_display)
      {
        std::stringstream image {settings->logo};
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
        client->comment(image.str () + output.str ());
      }
    else
      {
        if (settings->chan_notify.test (MessageType::State))
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
    Mpd::Client client {settings->mpd.host, settings->mpd.port};
    client.run_set_volume (settings->initial_volume);
    std::string last_file;
    bool init = true;
    while (true)
      {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for (1s);
        try
          {
            auto song = client.run_current_song ();
            auto file = song.uri ();
            if (init || file != last_file)
              {
                init = false;
                last_file = file;
                update_song (song);
              }
            AITHER_DEBUG("[displayinfo] update");
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
    pimpl->client = client ();
    pimpl->settings = &settings;
    pimpl->channel_message = [this] (const std::string &m)
    {
      channel_message (m);
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
    std::thread([&]
    {
      status_thread_proc ();
    });
  }

  std::string MpdPlugin::name ()
  {
    return "mpd";
  }

  /*
  def help(h)
    h << "<hr><span style='color:red;'>Plugin #{self.class.name}</span><br>"
    h << "<b>#{@@bot[:controlstring]}settings</b> - Print current MPD settings.<br>"
    h << "<b>#{@@bot[:controlstring]}seek <i>value</i> | <i>+/-value</i></b> - Seek to an absolute position (in seconds). Use +value or -value to seek relative to the current position.<br>"
    h << "<b>#{@@bot[:controlstring]}seek <i>mm:ss</i> | <i>+/-mm:ss</i></b> - Seek to an absolute position. Use + or - to seek relative to the current position.<br>"
    h << "<b>#{@@bot[:controlstring]}seek <i>hh:mm:ss</i> | <i>+/-hh:mm:ss</i></b> - Seek to an absolute position. Use + or - to seek relative to the current position.<br>"
    h << "<b>#{@@bot[:controlstring]}crossfade <i>value</i></b> - Set Crossfade to value seconds, 0 to disable crossfading.<br>"
    h << "<b>#{@@bot[:controlstring]}next</b> - Play next title in the queue.<br>"
    h << "<b>#{@@bot[:controlstring]}prev</b> - Play previous title in the queue.<br>"
    h << "<b>#{@@bot[:controlstring]}clear</b> - Clear the playqueue.<br>"
    h << "<b>#{@@bot[:controlstring]}random</b> - Toggle random mode.<br>"
    h << "<b>#{@@bot[:controlstring]}single</b> - Toggle single mode.<br>"
    h << "<b>#{@@bot[:controlstring]}repeat</b> - Toggle repeat mode.<br>"
    h << "<b>#{@@bot[:controlstring]}consume</b> - Toggle consume mode. If this mode is enabled, songs will be removed from the play queue once they were played.<br>"
    h << "<b>#{@@bot[:controlstring]}pp</b> - Toggle pause/play.<br>"
    h << "<b>#{@@bot[:controlstring]}stop</b> - Stop playing.<br>"
    h << "<b>#{@@bot[:controlstring]}play</b> - Start playing.<br>"
    h << "<b>#{@@bot[:controlstring]}play first</b> - Play the first song in the queue.<br>"
    h << "<b>#{@@bot[:controlstring]}play last</b> - Play the last song in the queue.<br>"
    h << "<b>#{@@bot[:controlstring]}play <i>number</i></b> - Play title on position <i>number</i> in queue.<br>"
    h << "<b>#{@@bot[:controlstring]}songlist</b> - Print the list of ALL songs in the MPD collection.<br>"
    h << "<b>#{@@bot[:controlstring]}playlist <i>id</i></b> - Load the playlist referenced by the id.<br>"
    h << "<b>#{@@bot[:controlstring]}saveplaylist <i>name</i></b> - Save queue into a playlist named 'name'<br>"
    h << "<b>#{@@bot[:controlstring]}delplaylist <i>id</i></b> - Remove a playlist with the given id. Use #{@@bot[:controlstring]}playlists to get a list of available playlists.<br>"
    h << "<b>#{@@bot[:controlstring]}song</b> - Print some information about the currently played song.<br>"
    h << "<b>#{@@bot[:controlstring]}status</b> - Print current status of MPD.<br>"
    h << "<b>#{@@bot[:controlstring]}playlists</b> - Print the available playlists from MPD.<br>"
    h << "<b>#{@@bot[:controlstring]}add <i>searchstring</i></b> - Find song(s) by searchstring and print matches.<br>"
    h << "<b>#{@@bot[:controlstring]}delete <i>ID</i></b> - Delete an entry from the current queue. Use #{@@bot[:controlstring]}queue to get the IDs of all songs in the current queue.<br>"
    h << "<b>#{@@bot[:controlstring]}where <i>searchstring</i></b> - Find song(s) by searchstring and print matches.<br>"
    h << "<b>#{@@bot[:controlstring]}queue</b> - Print the current play queue.<br>"
    h << "<b>#{@@bot[:controlstring]}stats</b> - Print some interesing MPD statistics.<br>"
    h << "<b>#{@@bot[:controlstring]}shuffle</b> – Play songs from the play queue in a random order.<br>"
    h << "<b>#{@@bot[:controlstring]}file</b> - Print the filename of the current song. This is useful if the file doesn't have ID3 tags and so the <b>#{@@bot[:controlstring]}song</b> command shows nothing.<br>"
    h << "<b>#{@@bot[:controlstring]}v++++</b> - Turns volume 20% up.<br>"
    h << "<b>#{@@bot[:controlstring]}v-</b> - Turns volume 5% down.<br>"
    h << "<b>#{@@bot[:controlstring]}v <i>value</i></b> - Set the volume to the given value.<br>"
    h << "<b>#{@@bot[:controlstring]}v</b> - Print the current playback volume.<br>"
    h << "<b>#{@@bot[:controlstring]}update</b> - Start a MPD database update.<br>"
    h << "<b>#{@@bot[:controlstring]}mpdconfig</b> - Try to read mpd config.<br>"
    h << "<b>#{@@bot[:controlstring]}mpdcommands</b> - Show what commands mpd do allow to Bot (not to you!).<br>"
    h << "<b>#{@@bot[:controlstring]}mpdnotcommands</b> - Show what commands mpd disallowed to Bot.<br>"
    h << "<b>#{@@bot[:controlstring]}mpddecoders</b> - Show enabled decoders and what they can decode for your mpd.<br>"
  end

  def handle_chat(msg,message)
    super
    if message == 'helpmpd'
        privatemessage( help(""))
    end

    if message == 'seek'
      # seek command without a value...
      privatemessage("Now on position #{timedecode @@bot[:mpd].status[:time][0]}/#{timedecode @@bot[:mpd].status[:time][1]}.")
    end

    if message[0..3] == 'seek'
      seekto = case message.count ":"
        when 0 then         # Seconds
          if message.match(/^seek [+-]?[0-9]{1,3}$/)
            result = message.match(/^seek ([+-]?[0-9]{1,3})$/)[1]
          else
            return 0
          end
        when 1 then         # Minutes:Seconds
          if message.match(/^seek ([+-]?[0-5]?[0-9]:[0-5]?[0-9])/)
            time = message.match(/^seek ([+-]?[0-5]?[0-9]:[0-5]?[0-9])/)[1].split(/:/)
            case time[0][0]
            when "+"
              result = time[0].to_i * 60 + time[1].to_i
              result = "+" + result.to_s
            when "-"
              result = time[0].to_i * 60 + time[1].to_i * -1
            else
              result = time[0].to_i * 60 + time[1].to_i
            end
          end
        when 2 then         # Hours:Minutes:Seconds
          if message.match(/^seek ([+-]?(?:[01]?[0-9]|2[0-3]):[0-5]?[0-9]:[0-5]?[0-9])/)
            time = message.match(/^seek ([+-]?(?:[01]?[0-9]|2[0-3]):[0-5]?[0-9]:[0-5]?[0-9])/)[1].split(/:/)
            case time[0][0]
            when "+"
              result = time[0].to_i * 3600 + time[1].to_i * 60 + time[2].to_i
              result = "+" + result.to_s
            when "-"
              result = time[0].to_i * 3600 + time[1].to_i * -60 + time[2].to_i * -1
            else
              result = time[0].to_i * 3600 + time[1].to_i * 60 + time[2].to_i
            end
          end
      end
      begin
        @@bot[:mpd].seek seekto
      rescue
        # mpd is old and knows no seek commands
        puts "[mpd-plugin] [error] seek without success, maybe mpd version < 0.17 installed"
      end
      channelmessage( "Now on position #{timedecode @@bot[:mpd].status[:time][0]}/#{timedecode @@bot[:mpd].status[:time][1]}.")
    end

    if message.match(/^crossfade [0-9]{1,3}$/)
      secs = message.match(/^crossfade ([0-9]{1,3})$/)[1].to_i
      @@bot[:mpd].crossfade = secs
    end

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

  private

  def timedecode(time)
    begin
      #Code from https://stackoverflow.com/questions/19595840/rails-get-the-time-difference-in-hours-minutes-and-seconds
      now_mm, now_ss = time.to_i.divmod(60)
      now_hh, now_mm = now_mm.divmod(60)
      if ( now_hh < 24 )
        now = "%02d:%02d:%02d" % [now_hh, now_mm, now_ss]
      else
        now_dd, now_hh = now_hh.divmod(24)
        now = "%04d days %02d:%02d:%02d" % [now_dd, now_hh, now_mm, now_ss]
      end
    rescue
      now "unknown"
    end
  end

  end
  */
}
