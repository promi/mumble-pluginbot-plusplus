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
      std::vector<CommandHelp> help;
      std::function<void (const CommandArgs&)> invoke;
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
    Command seek ();
    Command crossfade ();
    Command next ();
    Command prev ();
    Command clear ();
    Command random ();
    Command single ();
    Command repeat ();
    Command consume ();
    Command pp ();
    Command stop ();
    Command play ();
    Command songlist ();
    Command playlist ();
    Command saveplaylist ();
    Command delplaylist ();
    Command song ();
    Command status ();
    Command playlists ();
    Command add ();
    Command delete_ ();
    Command where ();
    Command queue ();
    Command stats ();
    Command shuffle ();
    Command file ();
    Command v ();
    Command update ();
    Command mpdconfig ();
    Command mpdcommands ();
    Command mpdnotcommands ();
    Command mpddecoders ();
    Command mpdurlhandlers ();
    Command displayinfo ();
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
            if (song == nullptr)
              {
                // TODO: Clear comment, etc.
              }
            else
              {
                auto file = song->uri ();
                if (init || file != last_file)
                  {
                    init = false;
                    last_file = file;
                    update_song (*song);
                    AITHER_DEBUG("[displayinfo] update");
                  }
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
      {"seek", seek ()},
      {"crossfade", crossfade ()},
      {"next", next ()},
      {"seek", seek ()},
      {"crossfade", crossfade ()},
      {"next", next ()},
      {"prev", prev ()},
      {"clear", clear ()},
      {"random", random ()},
      {"single", single ()},
      {"repeat", repeat ()},
      {"consume", consume ()},
      {"pp", pp ()},
      {"stop", stop ()},
      {"play", play ()},
      {"songlist", songlist ()},
      {"playlist", playlist ()},
      {"saveplaylist", saveplaylist ()},
      {"delplaylist", delplaylist ()},
      {"song", song ()},
      {"status", status ()},
      {"playlists", playlists ()},
      {"add", add ()},
      {"delete", delete_ ()},
      {"where", where ()},
      {"queue", queue ()},
      {"stats", stats ()},
      {"shuffle", shuffle ()},
      {"file", file ()},
      {"v", v ()},
      {"update", update ()},
      {"mpdconfig", mpdconfig ()},
      {"mpdcommands", mpdcommands ()},
      {"mpdnotcommands", mpdnotcommands ()},
      {"mpddecoders", mpddecoders ()},
      {"mpdurlhandlers", mpdurlhandlers ()},
      {"displayinfo", displayinfo ()}
    };
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

  MpdPlugin::Impl::Command MpdPlugin::Impl::seek ()
  {
    std::vector<CommandHelp> help =
    {
      {"value", "Seek to an absolute position (in seconds)."},
      {"+-value", "Seek relative to current position (in seconds)."},
      {"mm:ss", "Seek to an absolute position"},
      {"+/-mm:ss", "Seek relative to current position"},
      {"hh::mm:ss", "Seek to an absolute position"},
      {"+/-hh::mm:ss", "Seek relative to current position"}
    };
    auto invoke = [this] (auto ca)
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
                  throw std::invalid_argument ("Seconds out of range");
                }
              seconds = parts[0] * 60 + parts[1];
              break;
            case 3:
              if (parts[1] > 59)
                {
                  throw std::invalid_argument ("Minutes out of range");
                }
              if (parts[2] > 59)
                {
                  throw std::invalid_argument ("Seconds out of range");
                }
              seconds = parts[0] * 60 * 60 + parts[1] * 60 + parts[2];
              break;
            default:
              throw std::invalid_argument ("Invalid count of `:` chars");
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
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::crossfade ()
  {
    std::vector<CommandHelp> help =
    {
      {"value", "Set Crossfade to value seconds, 0 to disable crossfading."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.crossfade (std::stoi (ca.arguments));
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::next ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Play next title in the queue."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.next ();
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::prev ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Play previous title in the queue."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.previous ();
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::clear ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Clear the playqueue."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.clear ();
      private_message ("The play queue was cleared.");
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::random ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Toggle random mode."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.random (!ca.mpd_client.status ().random ());
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::single ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Toggle single mode."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.single (!ca.mpd_client.status ().single ());
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::repeat ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Toggle repeat mode."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.repeat (!ca.mpd_client.status ().repeat ());
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::consume ()
  {
    std::vector<CommandHelp> help =
    {
      {
        "", "Toggle consume mode. If this mode is enabled, songs will be "
        "removed from the play queue once they were played."
      }
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.consume (!ca.mpd_client.status ().consume ());
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::pp ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Toggle pause/play."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.toggle_pause ();
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::stop ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Stop playing."}
    };
    auto invoke = [this] (auto ca)
    {
      ca.mpd_client.stop ();
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::play ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Start playing."},
      {"first", "Play the first song in the queue."},
      {"last", "Play the last song in the queue."},
      {"number", "Play title on position <i>number</i> in queue."}
    };
    auto invoke = [this] (auto ca)
    {
      if (ca.arguments == "")
        {
          ca.mpd_client.play ();
        }
      else if (ca.arguments == "first")
        {
          try
            {
              ca.mpd_client.play_pos (0);
              private_message ("Playing first song in the queue (0).");
            }
          catch (...)
            {
              private_message ("There are no titles in the queue, "
                               "can't play the first entry.");
            }
        }
      else if (ca.arguments == "last")
        {
          size_t queue_length = 0;
          ca.mpd_client.send_list_queue_meta ();
          for (std::unique_ptr<Mpd::Song> song;
               (song = ca.mpd_client.recv_song ()) != nullptr; queue_length++);
          if (queue_length > 0)
            {
              auto last_song_id = queue_length - 1;
              ca.mpd_client.play_pos (last_song_id);
              private_message ("Playing last song in the queue (" +
                               std::to_string (last_song_id) + ").");
            }
          else
            {
              private_message("There are no titles in the queue, "
                              "can't play the last entry.");
            }
        }
      else
        {
          auto track_number = std::stoi (ca.arguments);
          try
            {
              ca.mpd_client.play_pos (track_number);
            }
          catch (...)
            {
              private_message("Title on position " + std::to_string (track_number) +
                              " does not exist");
            }
        }
      if (client.me ().deaf ())
        {
          client.deaf (false);
        }
      if (client.me ().mute ())
        {
          client.mute (false);
        }
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::songlist ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print the list of ALL songs in the MPD collection."}
    };
    auto invoke = [this] (auto ca)
    {
      const std::string &chunk_header = "<ul>\n";
      const std::string &chunk_footer = "</ul>\n";

      auto &mpd = ca.mpd_client;
      mpd.send_list_all ("");
      std::stringstream buffer;
      buffer << chunk_header;
      std::unique_ptr<Mpd::Song> song;
      // TODO: Even though the generated messages seem to be fine, only a certain amount
      // actually makes it to the Mumble client. Find out where the message is lost.
      for (size_t i = 0; (song = mpd.recv_song ()) != nullptr; i++)
        {
          buffer << li_tag (song->uri ());
          if ((i + 1) % 50 == 0)
            {
              buffer << chunk_footer;
              private_message (buffer.str ());
              buffer.clear ();
              buffer << chunk_header;
            }
        }
      if (buffer.str () != chunk_header)
        {
          buffer << chunk_footer;
          private_message (buffer.str ());
        }
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::playlist ()
  {
    std::vector<CommandHelp> help =
    {
      {"id", "Load the playlist referenced by the id."}
    };
    auto invoke = [this] (auto ca)
    {
      auto &mpd = ca.mpd_client;
      auto playlist_id = std::stoi (ca.arguments);
      try
        {
          auto playlists = mpd.playlists ();
          if (playlist_id < 0 || size_t (playlist_id) >= playlists.size ())
            {
              private_message ("A playlist with that id could not be found.");
            }
          else
            {
              auto playlist = std::move (playlists.at (playlist_id));
              auto path = playlist->path ();
              mpd.clear ();
              mpd.load (path);
              mpd.play ();
              private_message ("The playlist \"" + path +
                               "\" was loaded and is now playing.");
            }
        }
      catch (const std::runtime_error &e)
        {
          private_message ("A playlist with that id could not be loaded.");
          AITHER_DEBUG("Error loading playlist: " << playlist_id << ", " << e.what ());
        }
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::saveplaylist ()
  {
    std::vector<CommandHelp> help =
    {
      {"name", "Save queue into a playlist named 'name'"}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::delplaylist ()
  {
    auto controlstring = settings.controlstring;
    std::vector<CommandHelp> help =
    {
      {
        "id", "Remove a playlist with the given id. "
        "Use " + controlstring + "playlists to get a list of "
        "available playlists."
      }
    };
    auto invoke = [this] (auto ca)
    {
      /*
        playlist_id = message.match(/^delplaylist ([0-9]{1,3})$/)[1].to_i
        begin
        playlist = @@bot[:mpd].playlists[playlist_id]
        playlist.destroy
        privatemessage( "The playlist \"#{playlist.name}\" deleted.")
        rescue
        privatemessage( "Sorry, the given playlist id does not exist.")
        end
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::song ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print some information about the currently played song."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
              current = @@bot[:mpd].current_song
              if not current.nil? #Would crash if playlist was empty.
              privatemessage( "#{current.artist} - #{current.title} (#{current.album})")
              else
              privatemessage( "No song is played currently.")
              end
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::status ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print current status of MPD."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::playlists ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print the available playlists from MPD."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
        text_out = ""
        counter = 0
        @@bot[:mpd].playlists.each do |pl|
        text_out = text_out + "#{counter} - #{pl.name}<br/>"
        counter += 1
        end
        privatemessage( "I know the following playlists:<br>#{text_out}")
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::add ()
  {
    std::vector<CommandHelp> help =
    {
      {"searchstring", "Find song(s) by searchstring and print matches."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::delete_ ()
  {
    auto controlstring = settings.controlstring;
    std::vector<CommandHelp> help =
    {
      {
        "ID", "Delete an entry from the current queue. "
        "Use " + controlstring + "queue to get the IDs of all songs in "
        "the current queue."
      }
    };
    auto invoke = [this] (auto ca)
    {
      /*
        begin
        @@bot[:mpd].delete message.split(/ /)[1]
        rescue
        privatemessage( "Sorry, could not delete.")
        end
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::where ()
  {
    std::vector<CommandHelp> help =
    {
      {"searchstring", "Find song(s) by searchstring and print matches."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
        search = message.gsub("where", "").lstrip.tr('"\\','')
        text_out = "you should search not nothing!"
        if search != ""
        text_out ="found:<br/>"
        @@bot[:mpd].where(any: "#{search}").each do |song|
        text_out << "#{song.file}<br/>"
        end
        end
        privatemessage( text_out)
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::queue ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print the current play queue."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      privatemessage( text_out)
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::stats ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print some interesing MPD statistics."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::shuffle ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Play songs from the play queue in a random order."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
      @@bot[:mpd].shuffle
      privatemessage( "Shuffle, shuffle and get a new order. :)")
       */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::file ()
  {
    auto controlstring = settings.controlstring;
    std::vector<CommandHelp> help =
    {
      {
        "", "Print the filename of the current song. "
        "This is useful if the file doesn't have ID3 tags "
        "and so the " + controlstring + b_tag ("song") + " command shows nothing."
      }
    };
    auto invoke = [this] (auto ca)
    {
      /*
         current = @@bot[:mpd].current_song
         privatemessage( "Filename of currently played song:<br>#{current.file}</span>") if not current.nil?
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::v ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print the current playback volume."},
      {"value", "Set the volume to the given value."},
      {"++++", "Turns volume 20% up."},
      {"-", "Turns volume 5% down."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::update ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Start a MPD database update."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
              @@bot[:mpd].update
              privatemessage("Running database update...")
              while @@bot[:mpd].status[:updating_db] != nil do
              sleep 0.5
              end
              privatemessage("Done.")
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::mpdconfig ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Try to read mpd config."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
        config = @@bot[:mpd].config
        rescue
        config = "Configuration only for local clients readable"
        end
        privatemessage( config)
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::mpdcommands ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Show what commands mpd do allow to Bot (not to you!)."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
        output = ""
        @@bot[:mpd].commands.each do |command|
        output << "<br>#{command}"
        end
        privatemessage( output)
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::mpdnotcommands ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Show what commands mpd disallowed to Bot."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
        output = ""
        @@bot[:mpd].notcommands.each do |command|
        output << "<br\>#{command}"
        end
        privatemessage( output)
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::mpddecoders ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Show enabled decoders and what they can decode for your mpd."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::mpdurlhandlers ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Show mpd url handlers."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
        output = ""
        @@bot[:mpd].url_handlers.each do |handler|
        output << "<br\>#{handler}"
        end
        privatemessage( output)
      */
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::displayinfo ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
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
      */
    };
    return {help, invoke};
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
