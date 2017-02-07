/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 Natenom
    Copyright (c) 2015 netinetwalker
    Copyright (c) 2015 loscoala
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
#include "pluginbot/plugins/youtube.hh"

#include <chrono>

#include "mpd/client.hh"
#include "pluginbot/html.hh"
#include "pluginbot/plugins/command-help.hh"

namespace fs = std::experimental::filesystem;

namespace MumblePluginBot
{
  std::string squote (const std::string &s)
  {
    std::string result = "'";
    for (const char &c : s)
      {
        if (c == '\'')
          {
            result += "''";
          }
        else
          {
            result += c;
          }
      }
    return result + "'";
  };

  struct YoutubePlugin::Impl
  {
    struct CommandArgs
    {
      const MumbleProto::TextMessage &msg;
      uint32_t actor;
      const std::string &command;
      const std::string &arguments;
      std::function<void (const std::string&)> reply;
    };

    struct Command
    {
      std::vector<CommandHelp> help;
      std::function<void (const CommandArgs&)> invoke;
    };

    inline Impl (const Aither::Log &log, Settings &settings, Mumble::Client &client,
                 Mumble::AudioPlayer &player)
      : m_log (log), settings (settings), client (client), player (player)
    {
      init_commands ();
    }
    const Aither::Log &m_log;
    Settings &settings;
    Mumble::Client &client;
    Mumble::AudioPlayer &player;
    std::function<void (const std::string &)> channel_message;
    std::function<void (const std::string &)> private_message;
    std::function<void (uint32_t, const std::string &)> message_to;
    std::map<std::string, Command> m_commands;
    fs::path m_tempdir;
    fs::path m_targetdir;
    std::vector<std::string> m_filetypes;
    std::map<uint32_t, std::vector<std::pair<std::string, std::string>>>
    m_user_search_results;
    std::mutex m_user_search_results_mutex;
    void init_commands ();
    void link_thread_proc (const std::string &uri);
    void search_thread_proc (uint32_t user_id,
                             std::function<void (const std::string &)> reply, const std::string &search);
    Command link ();
    Command search ();
    Command add ();
    Command downloader_version ();
    std::string nice_add_exec (const std::string &cmd);
    std::string exec_ytdl (const std::string &arguments);
    std::string add_exec_ytdl (const std::string &arguments);
    std::string nice_exec_ytdl (const std::string &arguments);
    std::string nice_add_exec_ytdl (const std::string &arguments);
    std::vector<std::string> get_urls (const std::string &uri);
    std::vector<std::string> get_titles (const std::string &uri);
    std::vector<std::string> download (const std::string &uri);
    void resize_thumbnail (const fs::path &from, const fs::path &to);
    std::vector<std::string> convert_to_mp3 (const std::string &from,
        const std::string &to, const std::string &title);
    std::vector<std::string> tag (const std::string &from, const std::string &to,
                                  const std::string &title);
    std::vector<std::pair<std::string, std::string>> find_songs (
          const std::string &query);
    std::pair<std::vector<std::string>, std::vector<std::string>> get_songs (
          const std::string &uri);
  };

  YoutubePlugin::YoutubePlugin (const Aither::Log &log, Settings &settings,
                                Mumble::Client &cli,
                                Mumble::AudioPlayer &player)
    : Plugin (log, settings, cli, player),
      pimpl (std::make_unique<Impl> (this->m_log, this->settings (), this->client (),
                                     this->player ()))
  {
  }

  YoutubePlugin::~YoutubePlugin ()
  {
  }

  std::string YoutubePlugin::internal_name ()
  {
    return "youtube";
  }

  void YoutubePlugin::internal_init ()
  {
    pimpl->channel_message = [this] (const std::string &m)
    {
      channel_message (m);
    };
    pimpl->private_message = [this] (const std::string &m)
    {
      private_message (m);
    };
    pimpl->message_to = [this] (uint32_t user_id, const std::string &m)
    {
      message_to (user_id, m);
    };
    pimpl->m_tempdir = pimpl->settings.main_tempdir /
                       pimpl->settings.youtube.temp_subdir;
    fs::create_directories (pimpl->m_tempdir);
    pimpl->m_targetdir = pimpl->settings.mpd.musicdir /
                         pimpl->settings.youtube.download_subdir;
    fs::create_directories (pimpl->m_targetdir);
    pimpl->m_filetypes = {"ogg", "mp3", "mp2", "m4a", "aac", "wav", "ape", "flac", "opus"};
  }

  void YoutubePlugin::internal_chat (const MumbleProto::TextMessage &msg,
                                     const std::string &command,
                                     const std::string &arguments)
  {
    auto commands = pimpl->m_commands;
    auto cmd = commands.find (command);
    if (cmd == std::end (commands))
      {
        return;
      }
    auto actor = msg.actor ();
    auto reply = [this, actor] (const std::string &m)
    {
      message_to (actor, m);
    };
    Impl::CommandArgs ca {msg, actor, command, arguments, reply};
    cmd->second.invoke (ca);
  }

  std::string YoutubePlugin::internal_help ()
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

  void YoutubePlugin::Impl::init_commands ()
  {
    m_commands =
    {
      {"ytlink", link ()},
      {"yts", search ()},
      {"yta", add ()},
      {"ytdl-version", downloader_version ()}
    };
  }

  void YoutubePlugin::Impl::link_thread_proc (const std::string &uri)
  {
    Mpd::Client mpd {settings.mpd.host, settings.mpd.port};
    private_message ("Inspecting uri: " + uri + "...");
    if (settings.youtube.stream)
      {
        for (auto &url : get_urls (uri))
          {
            mpd.add (url);
          }
      }
    else
      {
        auto pair = get_songs (uri);
        auto &errors = pair.first;
        auto &songs = pair.second;
        for (auto &error : errors)
          {
            private_message (error);
          }
        if (songs.empty ())
          {
            private_message ("youtube-dl could not download anything from the given uri");
          }
        else
          {
            auto &subdir = settings.youtube.download_subdir;
            mpd.update (subdir);
            private_message ("Waiting for database update complete...");
            while (mpd.status ().updating_db ())
              {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for (500ms);
              }
            private_message ("Update done.");
            for (const auto &song : songs)
              {
                private_message (song);
                mpd.add (subdir + "/" + song);
              }
          }
      }
  }

  YoutubePlugin::Impl::Command YoutubePlugin::Impl::link ()
  {
    std::vector<CommandHelp> help =
    {
      {"url", "Download the music from the given url."}
    };
    auto invoke = [this] (auto ca)
    {
      auto uri = strip_tags (ca.arguments);
      std::thread t
      {
        [this, uri] ()
        {
          try
            {
              link_thread_proc (uri);
            }
          catch (const std::exception &e)
            {
              private_message (e.what ());
            }
        }
      };
      t.detach ();
    };
    return {help, invoke};
  }

  void YoutubePlugin::Impl::search_thread_proc (uint32_t user_id,
      std::function<void (const std::string &)> reply, const std::string &search)
  {
    reply ("searching for \"" + search + "\", please be patient...");
    auto songs = find_songs (squote (search));
    std::lock_guard<std::mutex> lock (m_user_search_results_mutex);
    m_user_search_results[user_id] = songs;
    std::stringstream out;
    for (size_t i = 0; i < songs.size (); i++)
      {
        auto &song = songs[i];
        if ((i % 30) == 0)
          {
            if (i != 0)
              {
                reply (out.str () + "</table>");
              }
            out.str ("<table><tr><td><b>Index</b></td><td>Title</td></tr>");
          }
        out << "<tr><td><b>" << song.first << "</b></td><td>" << song.second <<
            "</td></tr>";
      }
    out << "</table>";
    reply (out.str ());
  }

  YoutubePlugin::Impl::Command YoutubePlugin::Impl::search ()
  {
    std::vector<CommandHelp> help =
    {
      {"keywords", "Search on YouTube for one or more keywords and print the results"}
    };
    auto invoke = [this] (auto ca)
    {
      auto &actor = ca.actor;
      auto &search = ca.arguments;
      auto reply = ca.reply;
      if (search == "")
        {
          reply ("Please enter a search string.");
          return;
        }
      std::thread t { [this, actor, reply, search] ()
      {
        search_thread_proc (actor, reply, search);
      }
                    };
      t.detach ();
    };
    return {help, invoke};
  }

  YoutubePlugin::Impl::Command YoutubePlugin::Impl::add ()
  {
    const std::string &controlstring = settings.controlstring;
    std::vector<CommandHelp> help =
    {
      {
        "number number2 ... numberN", "Download the given song(s) from the list you got"
        " via " + i_tag (controlstring + "yts") + "."
      },
      {"all", "Download all found songs."}
    };
    auto invoke = [this] (auto ca)
    {
      /*
      if message.split[0] == 'yta'
      begin
      out = "<br>Going to download the following songs:<br />"
      msg_parameters = message.split[1..-1].join(" ")
      link = []

      if msg_parameters.match(/(?:[\d{1,3}\ ?])+/) # User gave us at least one id or multiple ids to download.
        id_list = msg_parameters.match(/(?:[\d{1,3}\ ?])+/)[0].split
        id_list.each do |id|
          downloadid = @keylist[msg.actor][id.to_i]
          puts downloadid.inspect
          out << "ID: #{id}, Name: \"#{downloadid[1]}\"<br>"
          link << "https://www.youtube.com/watch?v="+downloadid[0]
        end

        messageto(msg.actor, out)
      end

      if msg_parameters == "all"
        @keylist[msg.actor].each do |downloadid|
          out << "Name: \"#{downloadid[1]}\"<br>"
          link << "https://www.youtube.com/watch?v="+downloadid[0]
        end
        messageto(msg.actor, out)
      end
      rescue
      messageto(msg.actor, "[error](youtube-plugin)- index number is out of bounds!")
      end

      workingdownload = Thread.new {
      #local variables for this thread!
      actor = msg.actor
      Thread.current["user"]=actor
      Thread.current["process"]="youtube (yta)"

      messageto(actor, "do #{link.length.to_s} time(s)...")
      link.each do |l|
          messageto(actor, "fetch and convert")
          get_song(l).each do |error|
              @@bot[:messages.text(actor, error)]
          end
      end
      if ( @songlist.size > 0 ) then
        @@bot[:mpd].update(@@bot[:youtube_downloadsubdir].gsub(/\//,""))
        messageto(actor, "Waiting for database update complete...")

        while @@bot[:mpd].status[:updating_db] != nil do
          sleep 0.5
        end

        messageto(actor, "Update done.")
        out = "<b>Added:</b><br>"

        while @songlist.size > 0
          song = @songlist.pop
          begin
            @@bot[:mpd].add(@@bot[:youtube_downloadsubdir]+song)
            out << song + "<br>"
          rescue
            out << "fixme: " + song + " not found!<br>"
          end
        end
        messageto(actor, out)
      else
        messageto(actor, "Youtube: The link contains nothing interesting.") if @@bot[:youtube_stream] == nil
      end
      }
      end
      end
      */
    };
    return {help, invoke};
  }

  YoutubePlugin::Impl::Command YoutubePlugin::Impl::downloader_version ()
  {
    std::vector<CommandHelp> help =
    {
      {"", "Print download helper (youtube-dl) version"}
    };
    auto invoke = [this] (auto ca)
    {
      (void) ca;
      private_message ("youtube-dl version: " + exec_ytdl ("--version"));
    };
    return {help, invoke};
  }

  std::string exec (const std::string &cmd)
  {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, int(*)(FILE*)> pipe (popen (cmd.c_str (), "r"), pclose);
    if (pipe == nullptr)
      {
        throw std::runtime_error ("popen() failed");
      }
    while ((fgets (buffer.data (), buffer.size (), pipe.get ())) != nullptr)
      {
        result += buffer.data ();
      }
    return result;
  }

  std::string intercalate (const std::vector<std::string> &vector,
                           const char delimiter = ' ')
  {
    std::string result;
    bool first = true;
    for (const auto &s : vector)
      {
        if (first)
          {
            result = s;
            first = false;
          }
        else
          {
            result += delimiter + s;
          }
      }
    return result;
  }

  std::string nice_exec (const std::string &cmd)
  {
    return exec (intercalate ({"nice", "-n20", cmd}));
  }

  std::string YoutubePlugin::Impl::nice_add_exec (const std::string &cmd)
  {
    return exec (intercalate ({"nice", "-n20", settings.youtube.command_line_prefixes, cmd}));
  }

  std::string YoutubePlugin::Impl::exec_ytdl (const std::string &arguments)
  {
    return exec (intercalate ({settings.youtube.youtubedl, arguments}));
  }

  std::string YoutubePlugin::Impl::add_exec_ytdl (const std::string &arguments)
  {
    return exec (intercalate ({settings.youtube.command_line_prefixes, settings.youtube.youtubedl, arguments}));
  }

  std::string YoutubePlugin::Impl::nice_exec_ytdl (const std::string &arguments)
  {
    return nice_exec (intercalate ({settings.youtube.youtubedl, arguments}));
  }

  std::string YoutubePlugin::Impl::nice_add_exec_ytdl (const std::string
      &arguments)
  {
    return nice_exec (intercalate ({settings.youtube.command_line_prefixes, settings.youtube.youtubedl, arguments}));
  }

  std::vector<std::pair<std::string, std::string>>
      YoutubePlugin::Impl::find_songs (const std::string &query)
  {
    std::vector<std::pair<std::string, std::string>> v;
    std::stringstream output {nice_exec_ytdl (intercalate (
      {
        "--max-downloads " + std::to_string (settings.youtube.max_results),
        "--get-id",
        "--get-title",
        "https://www.youtube.com/results?search_query=" + query
      }))
    };
    std::string title;
    for (std::string id; std::getline (output, id); )
      {
        std::getline (output, title);
        v.push_back (std::make_pair (id, title));
      }
    return v;
  }

  void YoutubePlugin::Impl::resize_thumbnail (const fs::path &from,
      const fs::path &to)
  {
    nice_add_exec (intercalate (
    {
      "convert",
      squote (from),
      "-resize 320x240",
      squote (to)
    }));
  }

  std::vector<std::string> YoutubePlugin::Impl::get_titles (
    const std::string &uri)
  {
    std::vector<std::string> v;
    std::stringstream output {exec_ytdl (intercalate ({
        "--get-filename",
        settings.youtube.youtubedl_options,
        "--ignore-errors",
        "--output '%(title)s'",
        uri
      }))
    };
    for (std::string line; std::getline (output, line); )
      {
        v.push_back (line);
      }
    return v;
  }

  std::vector<std::string> YoutubePlugin::Impl::download (const std::string &uri)
  {
    std::vector<std::string> v;
    std::stringstream output {nice_add_exec_ytdl (intercalate({
        settings.youtube.youtubedl_options,
        "--write-thumbnail",
        "--extract-audio",
        "--audio-format best",
        "--output " + squote (m_tempdir.string () + "/%(title)s.%(ext)s"),
        uri,
        "2>&1"
      }))
    };
    for (std::string line; std::getline (output, line); )
      {
        v.push_back (line);
      }
    return v;
  }

  std::vector<std::string> YoutubePlugin::Impl::convert_to_mp3 (
    const std::string &from, const std::string &to, const std::string &title)
  {
    std::vector<std::string> v;
    std::stringstream output {nice_add_exec (intercalate ({
        "ffmpeg",
        "-n",
        "-i " + squote (from),
        "-codec:a libmp3lame",
        "-qscale:a 2",
        "-metadata title=" + squote (title),
        squote (to),
        "2>&1"
      }))
    };
    for (std::string line; std::getline (output, line); )
      {
        v.push_back (line);
      }
    return v;
  }

  std::vector<std::string> YoutubePlugin::Impl::tag (const std::string &from,
      const std::string &to, const std::string &title)
  {
    std::vector<std::string> v;
    std::stringstream output {nice_add_exec (intercalate (
      {
        "ffmpeg",
        "-n",
        "-i " + squote (from),
        "-codec:a copy",
        "-metadata title=" + squote (title),
        squote (to),
        "2>&1"
      }))
    };
    for (std::string line; std::getline (output, line); )
      {
        v.push_back (line);
      }
    return v;
  }

  std::vector<std::string> YoutubePlugin::Impl::get_urls (const std::string &uri)
  {
    std::vector<std::string> urls;
    std::stringstream streams {exec_ytdl ("--get-url " + uri)};
    for (std::string stream; std::getline (streams, stream); )
      {
        if (stream.find ("mime=audio/mp4") != std::string::npos)
          {
            urls.push_back (stream);
          }
      }
    return urls;
  }

  std::pair<std::vector<std::string>, std::vector<std::string>>
      YoutubePlugin::Impl::get_songs (const std::string &uri)
  {
    std::pair<std::vector<std::string>, std::vector<std::string>> pair;
    std::vector<std::string> &errors = pair.first;
    std::vector<std::string> &songs = pair.second;
    auto titles = get_titles (uri);
    auto output = download (uri);
    for (const auto &line : output)
      {
        if (line.find ("ERROR:") != std::string::npos)
          {
            errors.push_back (line);
          }
      }
    for (const auto &title : titles)
      {
        for (const auto &filetype : m_filetypes)
          {
            fs::path audio_temp_path = m_tempdir / (title + "." + filetype);
            fs::path audio_target_path = m_targetdir / (title + "." + filetype);
            fs::path mp3_target_path = m_targetdir / (title + ".mp3");
            fs::path jpg_temp_path = m_tempdir / (title + ".jpg");
            fs::path jpg_target_path = m_targetdir / (title + ".jpg");
            if (fs::exists (audio_temp_path))
              {
                resize_thumbnail (jpg_temp_path, jpg_target_path);
                if (settings.youtube.convert_to_mp3)
                  {
                    // Mixin tags and recode it to mp3 (vbr 190kBit)
                    convert_to_mp3 (audio_temp_path, mp3_target_path, title);
                    songs.push_back (mp3_target_path.filename ().string ());
                  }
                else
                  {
                    // Mixin tags without recode on standard
                    tag (audio_temp_path, audio_target_path, title);
                    songs.push_back (audio_target_path.filename ().string ());
                  }
              }
          }
      }
    return pair;
  }
}
