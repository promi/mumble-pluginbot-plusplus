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
    void inner_status_update (const FlagSet<Mpd::Idle> &idle_flags);
    void idle_thread_proc ();
    void update_comment (Mpd::Song *song);
    void update_song (Mpd::Song *song);
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
    std::unique_ptr<Mpd::Playlist> playlist_by_id (Mpd::Client &mpd,
        int playlist_id);
  };

  std::string time_decode (uint time);
  std::vector<uint> split_timecode (const std::string &timecode);
  std::string song_display_text (Mpd::Song &song);
  std::string song_display_text (Mpd::Song *song);
  std::string on_off (bool b);
  std::string state_display_text (Mpd::State state);

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
  }

  void MpdPlugin::Impl::status_update (const FlagSet<Mpd::Idle> &idle_flags)
  {
    try
      {
        inner_status_update (idle_flags);
      }
    catch (std::runtime_error &e)
      {
        channel_message (red_bold_span (std::string {"An error occured: "} +
                                        e.what ()));
      }
  }

  void MpdPlugin::Impl::inner_status_update (const FlagSet<Mpd::Idle> &idle_flags)
  {
    Mpd::Client client {settings.mpd.host, settings.mpd.port};
    auto status = client.status ();
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
            channel_message ("A MPD database update has finished");
            // TODO: What is a jobid, check where ruby-mpd got it if relevant.
            // "<br>The job id is: #{jobid}."
          }
      }
    if (idle_flags.test (Mpd::Idle::Options))
      {
        auto chan_notify = settings.chan_notify;
        if (chan_notify.test (MessageType::Random))
          {
            channel_message (std::string {"Random mode is now: "} +
                             on_off (status.random ()));
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
                             on_off (status.single ()));
          }
        if (chan_notify.test (MessageType::Consume))
          {
            channel_message (std::string {"Consume mode is now: "} +
                             on_off (status.consume ()));
          }
        if (chan_notify.test (MessageType::XFade))
          {
            auto xfade = status.crossfade ();
            if (xfade == 0)
              {
                channel_message ("Crossfade is now: " + on_off (false));
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
                             on_off (status.repeat ()));
          }
      }
    if (idle_flags.test (Mpd::Idle::Player))
      {
        static std::string last_file;
        auto song = client.current_song ();
        std::string file;
        if (song != nullptr)
          {
            file = song->uri ();
          }
        if (file != last_file)
          {
            last_file = file;
            update_song (song.get ());
            AITHER_DEBUG("Song changed");
          }
      }
  }

  void MpdPlugin::Impl::update_comment (Mpd::Song *song)
  {
    std::stringstream output;
    if (song != nullptr)
      {
        auto artist = song->tag (Mpd::TagType::Artist, 0);
        auto title = song->tag (Mpd::TagType::Title, 0);
        auto album = song->tag (Mpd::TagType::Album, 0);
        auto file = song->uri ();
        // TODO:
        //
        // 1. Find a better way to get the current song image file that does not require
        // this plugin to know so much about how the youtube, etc. plugins store their
        // images.
        //
        // 2. The image combined with the text may be too big to display if the default
        // comment size limit is set.
        //
        // Either make displaying the image optional or detect the maximum comment size
        // and act accordingly.
        /*
          std::stringstream image {settings.logo};
          if ( @@bot[:youtube_downloadsubdir] != nil ) && ( @@bot[:mpd_musicfolder] != nil )
          if File.exist?(@@bot[:mpd_musicfolder]+current.file.to_s.chomp(File.extname(current.file.to_s))+".jpg")
          image = @@bot[:cli].get_imgmsg(@@bot[:mpd_musicfolder]+current.file.to_s.chomp(File.extname(current.file.to_s))+".jpg")
          end
          output << image << "<br>\n";
        */
        output << "<table>\n";
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
        output << "</table>\n";
        output << "<br>\n";
      }
    output << info_template;
    client.comment (output.str ());
  }

  void MpdPlugin::Impl::update_song (Mpd::Song *song)
  {
    if (settings.use_comment_for_status_display)
      {
        update_comment (song);
      }
    else
      {
        if (song != nullptr)
          {
            channel_message (song_display_text (song));
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
    Mpd::Client client {settings.mpd.host, settings.mpd.port};
    client.volume (settings.initial_volume);
    pimpl->update_song (client.current_song ().get ());
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
      auto playlist = playlist_by_id (mpd, std::stoi (ca.arguments));
      if (playlist == nullptr)
        {
          return;
        }
      auto path = playlist->path ();
      mpd.clear ();
      mpd.load (path);
      mpd.play ();
      private_message ("The playlist \"" + path +
                       "\" was loaded and is now playing.");
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
      auto name = ca.arguments;
      if (name == "")
        {
          private_message ("Please specify a playlist name.");
        }
      else
        {
          AITHER_DEBUG (name);
          ca.mpd_client.save (name);
          private_message ("The playlist \"" + name + "\" was created. Use the command " +
                           settings.controlstring + "playlists to get a list of " +
                           "all available playlists.");
        }
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
      auto &mpd = ca.mpd_client;
      auto playlist = playlist_by_id (mpd, std::stoi (ca.arguments));
      if (playlist == nullptr)
        {
          return;
        }
      auto path = playlist->path ();
      mpd.rm (path);
      private_message ("The playlist \"" + path + "\" was deleted.");
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
      auto song = ca.mpd_client.current_song ();
      if (song == nullptr)
        {
          private_message ("No song is currently playing.");
        }
      else
        {
          private_message (song_display_text (*song));
        }
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
      auto &mpd = ca.mpd_client;
      auto status = mpd.status ();
      auto audio_format = status.audio_format ();
      std::string sample_rate;
      std::string bits;
      std::string channels;
      if (audio_format)
        {
          sample_rate = std::to_string (audio_format->sample_rate ());
          bits = std::to_string (audio_format->bits ());
          channels = std::to_string (audio_format->channels ());
        }
      std::stringstream out {"<table>\n"};
      auto row = [&] (const std::string key, const std::string value)
      {
        out << tr_tag (td_tag (key) + td_tag (value));
      };
      row ("Volume", std::to_string (status.volume ()) + "%");
      row ("Repeat", on_off (status.repeat ()));
      row ("Random", on_off (status.random ()));
      row ("Single", on_off (status.single ()));
      row ("Consume", on_off (status.consume ()));
      row ("Queue song count", std::to_string (status.queue_length ()));
      row ("Mixramp", std::to_string (status.mixrampdb ()) + " dB");
      row ("State", state_display_text (status.state ()));
      row ("Song", song_display_text (mpd.current_song ().get ()));
      row ("Song ID", std::to_string (status.song_id ()));
      row ("Position", time_decode (status.elapsed_time ()) +
           "/" + time_decode (status.total_time ()));
      row ("Bitrate", std::to_string (status.kbit_rate ()) + " kbit");
      row ("Samplerate", sample_rate);
      row ("Bits", bits);
      row ("Channels", channels);
      row ("Next song", song_display_text (mpd.get_queue_song_id (
                                             status.next_song_id ()).get ()));
      row ("Next song ID", std::to_string (status.next_song_id ()));
      out << "</table>\n";
      private_message (out.str ());
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
      std::stringstream ss {"Playlists<br/>\n<table>\n"};
      auto &mpd = ca.mpd_client;
      mpd.send_list_playlists ();
      auto playlists = mpd.recv_playlists ();
      for (size_t i = 0; i < playlists.size (); i++)
        {
          ss << tr_tag (td_tag (std::to_string (i)) + td_tag (playlists[i]->path ()));
        }
      ss << "</table>\n";
      private_message (ss.str ());
    };
    return {help, invoke};
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::add ()
  {
    std::vector<CommandHelp> help =
    {
      {"searchstring", "Find songs by searchstring and add them to the queue."}
    };
    auto invoke = [this] (auto ca)
    {
      // TODO:
      //
      // The original ruby pluginbot sends a reply which files where found and added.
      // While the user can test that with the 'where' command, it might be useful
      // to reimplement that here as well.
      //
      // On the other hand search_add_db_songs conveniently auto adds to queue.
      auto &mpd = ca.mpd_client;
      mpd.search_add_db_songs (false);
      mpd.search_add_any_tag_constraint (Mpd::Operator::Default, ca.arguments);
      mpd.search_commit ();
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
      ca.mpd_client.delete_ (std::stoi (ca.arguments));
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
      auto &mpd = ca.mpd_client;
      mpd.search_db_songs (false);
      mpd.search_add_any_tag_constraint (Mpd::Operator::Default, ca.arguments);
      mpd.search_commit ();
      auto songs = mpd.recv_songs ();
      if (songs.size () == 0)
        {
          private_message ("No songs found.");
        }
      else
        {
          std::stringstream ss {"<ul>\n"};
          for (auto &song : songs)
            {
              ss << li_tag (song->uri ());
            }
          ss << "</ul>";
          private_message (ss.str ());
        }
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
      auto &mpd = ca.mpd_client;
      mpd.send_list_queue_meta ();
      auto songs = mpd.recv_songs ();
      if (songs.empty ())
        {
          private_message ("The queue is empty.");
          return;
        }
      std::stringstream ss;
      ss << th_tag (td_tag ("#") + td_tag ("Name"));
      for (size_t i = 0; i < songs.size (); i++)
        {
          ss << tr_tag (td_tag (std::to_string (i) +
                                td_tag (song_display_text (songs[i].get ()))));
        }
      private_message (table_tag (ss.str ()));
    };
    return {help, invoke};
  }

  std::string trow (const std::string &key, unsigned long value)
  {
    return srow (key, time_decode (value));
  }

  MpdPlugin::Impl::Command MpdPlugin::Impl::stats ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print some interesing MPD statistics."}
    };
    auto invoke = [this] (auto ca)
    {
      auto stats = ca.mpd_client.stats ();
      std::stringstream ss;
      ss << urow ("Number of artists", stats.number_of_artists ());
      ss << urow ("Number of albums", stats.number_of_albums ());
      ss << urow ("Number of songs", stats.number_of_songs ());
      ss << trow ("Uptime", stats.uptime ());
      ss << trow ("DB update time", stats.db_update_time ());
      ss << trow ("Play time", stats.play_time ());
      ss << trow ("DB play time", stats.db_play_time ());
      private_message (table_tag (ss.str ()));
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
      ca.mpd_client.shuffle ();
      private_message ("Shuffle, shuffle and get a new order. :)");
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
      auto song = ca.mpd_client.current_song ();
      if (song != nullptr)
        {
          private_message ("Filename of the currently playing song:" + br_tag +
                           song->uri () + "\n");
        }
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
      auto &mpd = ca.mpd_client;
      if (ca.arguments.empty ())
        {
        }
      else
        {
          if (ca.arguments[0] == '-')
            {
              mpd.change_volume ((-5) * ca.arguments.size ());
            }
          else if (ca.arguments[0] == '+')
            {
              mpd.change_volume (5 * ca.arguments.size ());
            }
          else
            {
              mpd.volume (std::stoi (ca.arguments));
            }
        }
      auto volume = std::to_string (mpd.status ().volume ());
      private_message ("Current volume is " + volume + "%.");
      /*
      if (volume >=0 ) && (volume <= 100)
      else
      privatemessage( "Volume can be within a range of 0 to 100")
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
      {"Toggle sending song info to comment or channel.", "."}
    };
    auto invoke = [this] (auto ca)
    {
      (void) ca;
      std::string target;
      if ((settings.use_comment_for_status_display =
             !settings.use_comment_for_status_display))
        {
          target = "Comment";
        }
      else
        {
          target = "Channel";
        }
      update_comment (nullptr);
      private_message ("Song info updates are now send to: " + target);
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

  std::string time_decode (uint time)
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

  std::vector<uint> split_timecode (const std::string &timecode)
  {
    std::stringstream ss {timecode};
    std::vector<uint> v;
    for (std::string part; std::getline (ss, part, ':'); )
      {
        v.push_back (std::stoi (part));
      }
    return v;
  }

  std::unique_ptr<Mpd::Playlist> MpdPlugin::Impl::playlist_by_id (
    Mpd::Client &mpd,
    int playlist_id)
  {
    mpd.send_list_playlists ();
    auto playlists = mpd.recv_playlists ();
    if (playlist_id < 0 || size_t (playlist_id) >= playlists.size ())
      {
        private_message ("A playlist with that id does not exist.");
        return nullptr;
      }
    else
      {
        return std::move (playlists.at (playlist_id));
      }
  }

  std::string song_display_text (Mpd::Song& song)
  {
    auto artist = song.tag (Mpd::TagType::Artist, 0);
    auto title = song.tag (Mpd::TagType::Title, 0);
    auto album = song.tag (Mpd::TagType::Album, 0);

    if (artist != nullptr && title != nullptr && album != nullptr)
      {
        return *artist + " - " + *title + " (" + *album + ")";
      }
    else if (artist != nullptr && title != nullptr)
      {
        return *artist + " - " + *title;
      }
    else
      {
        return song.uri ();
      }
  }

  std::string song_display_text (Mpd::Song* song)
  {
    if (song == nullptr)
      {
        return "";
      }
    else
      {
        return song_display_text (*song);
      }
  }

  std::string on_off (bool b)
  {
    return b ? "On" : "Off";
  }

  std::string state_display_text (Mpd::State state)
  {
    switch (state)
      {
      case Mpd::State::Stop:
        return "Stopped";
      case Mpd::State::Play:
        return "Playing";
      case Mpd::State::Pause:
        return "Paused";
      default:
        return "Unknown";
      }
  }
}
