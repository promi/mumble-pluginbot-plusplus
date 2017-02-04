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

#include "mpd/client.hh"
#include "pluginbot/html.hh"
#include "pluginbot/plugins/command-help.hh"

namespace MumblePluginBot
{
  struct YoutubePlugin::Impl
  {
    struct CommandArgs
    {
      const MumbleProto::TextMessage &msg;
      const std::string &command;
      const std::string &arguments;
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
    std::map<std::string, Command> m_commands;
    void init_commands ();
    Command link ();
    Command search ();
    Command add ();
    Command downloader_version ();
    std::string exec_ytdl (const std::string &arguments);
    std::string add_exec_ytdl (const std::string &arguments);
    std::string nice_exec_ytdl (const std::string &arguments);
    std::string nice_add_exec_ytdl (const std::string &arguments);
    std::vector<std::pair<std::string, std::string>> find_songs (
          const std::string &query);
    std::pair<std::vector<std::string>, std::vector<std::string>> get_songs (
          const std::string &uri);
  };

  std::string exec (const std::string &cmd);

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
    /*
      begin
        @youtubefolder = @@bot[:mpd_musicfolder] + @@bot[:youtube_downloadsubdir]
        @tempyoutubefolder = @@bot[:main_tempdir] + @@bot[:youtube_tempsubdir]

        Dir.mkdir(@youtubefolder) unless File.exists?(@youtubefolder)
        Dir.mkdir(@tempyoutubefolder) unless File.exists?(@tempyoutubefolder)
      rescue
        puts "Error: Youtube-Plugin didn't find settings for mpd music directory and/or your preferred temporary download directory"
        puts "See pluginbot_conf.rb"
      end
      begin
        @ytdloptions = @@bot[:youtube_youtubedl_options]
      rescue
        @ytdloptions = ""
      end
      @consoleaddition = ""
      @consoleaddition = @@bot[:youtube_commandlineprefixes] if @@bot[:youtube_commandlineprefixes] != nil
      @songlist = Queue.new
      @keylist = Array.new
      @@bot[:youtube] = self
    end
    @filetypes= ["ogg", "mp3", "mp2", "m4a", "aac", "wav", "ape", "flac", "opus"]
    */
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
    Impl::CommandArgs ca {msg, command, arguments};
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

  YoutubePlugin::Impl::Command YoutubePlugin::Impl::link ()
  {
    std::vector<CommandHelp> help =
    {
      {"url", "Download the music from the given url."}
    };
    auto invoke = [this] (auto ca)
    {
      auto link = strip_tags (ca.arguments);
      std::thread t {[this, link] ()
      {
        private_message ("Inspecting link: " + link + "...");
        auto pair = get_songs (link);
        auto &errors = pair.first;
        auto songs = pair.second;
        for (auto &error : errors)
          {
            private_message (error);
          }

        /*
          if ( @songlist.size > 0 ) then
          @@bot[:mpd].update(@@bot[:youtube_downloadsubdir].gsub(/\//,""))
          messageto(actor, "Waiting for database update complete...")

          while @@bot[:mpd].status[:updating_db] != nil do
          sleep 0.5
          end

          messageto(actor, "Update done.")
          while @songlist.size > 0
          song = @songlist.pop
          messageto(actor, song)
          @@bot[:mpd].add(@@bot[:youtube_downloadsubdir]+song)
          end
          else
          messageto(actor, "Youtube: The link contains nothing interesting.") if @@bot[:youtube_stream] == nil
          end
        */
      }
                    };
      t.detach ();
    };
    return {help, invoke};
  }

  YoutubePlugin::Impl::Command YoutubePlugin::Impl::search ()
  {
    std::vector<CommandHelp> help =
    {
      {"keywords", "Search on YouTube for one or more keywords and print the results"}
    };
    auto invoke = [this] (auto ca)
    {
      /*
      if message.split[0] == 'yts'
      search = message[4..-1]
      if !(( search == nil ) || ( search == "" ))
      Thread.new do
        Thread.current["user"]=msg.actor
        Thread.current["process"]="youtube (yts)"

        messageto(msg.actor, "searching for \"#{search}\", please be patient...")
        songs = find_youtube_song(CGI.escape(search))
        @keylist[msg.actor] = songs
        index = 0
        out = ""
        @keylist[msg.actor].each do |id , title|
          if ( ( index % 30 ) == 0 )
            messageto(msg.actor, out + "</table>") if index != 0
            out = "<table><tr><td><b>Index</b></td><td>Title</td></tr>"
          end
          out << "<tr><td><b>#{index}</b></td><td>#{title}</td></tr>"
          index += 1
        end
        out << "</table>"
        messageto(msg.actor, out)
      end
      else
      messageto(msg.actor, "won't search for nothing!")
      end
      end
       */
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
    std::vector<std::pair<std::string, std::string>> songlist;
    /*
    songs = `nice -n20 #{@@bot[:youtube_youtubedl]} --max-downloads #{@@bot[:youtube_maxresults]} --get-title --get-id "https://www.youtube.com/results?search_query=#{song}"`
    temp = songs.split(/\n/)
    while (temp.length >= 2 )
      songlist << [temp.pop , temp.pop]
    end
     */
    return songlist;
  }

  namespace fs = std::experimental::filesystem;

  std::pair<std::vector<std::string>, std::vector<std::string>>
      YoutubePlugin::Impl::get_songs (const std::string &uri)
  {
    std::pair<std::vector<std::string>, std::vector<std::string>> pair;
    std::vector<std::string> &errors = pair.first;
    std::vector<std::string> &songlist = pair.second;
    if (settings.youtube.stream)
      {
        std::stringstream streams {exec_ytdl ("--get-url " + uri)};
        Mpd::Client mpd {settings.mpd.host, settings.mpd.port};
        for (std::string stream; std::getline (streams, stream); )
          {
            if (stream.find ("mime=audio/mp4") != std::string::npos)
              {
                mpd.add (stream);
              }
          }
      }
    else
      {
        auto squote = [] (const std::string &s)
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
        fs::path tempdir = settings.main_tempdir;
        tempdir /= settings.youtube.temp_subdir;
        fs::create_directories (tempdir);
        std::stringstream filenames {exec_ytdl (intercalate ({
            "--get-filename",
            settings.youtube.youtubedl_options,
            "--ignore-errors",
            "--output " + squote (tempdir.string () + "/%(title)s"),
            uri
          }))
        };
        std::stringstream output {nice_add_exec_ytdl (intercalate({
            settings.youtube.youtubedl_options,
            "--write-thumbnail",
            "--extract-audio",
            "--audio-format best",
            "--output " + squote (tempdir.string () + "/%(title)s.%(ext)s"),
            uri
          }))
        };
        for (std::string line; std::getline (output, line); )
          {
            if (line.find ("ERROR:") != std::string::npos)
              {
                errors.push_back (line);
              }
          }
        for (std::string filename; std::getline (filenames, filename); )
          {
            /*
            @filetypes.each do |ending|
              if File.exist?("#{@tempyoutubefolder}#{name}.#{ending}")
                system ("nice -n20 #{@consoleaddition} convert \"#{@tempyoutubefolder}#{name}.jpg\" -resize 320x240 \"#{@youtubefolder}#{name}.jpg\" ")
                if @@bot[:youtube_to_mp3] == nil
                  # Mixin tags without recode on standard
                  system ("nice -n20 #{@consoleaddition} ffmpeg -i \"#{@tempyoutubefolder}#{name}.#{ending}\" -acodec copy -metadata title=\"#{name}\" \"#{@youtubefolder}#{name}.#{ending}\"") if !File.exist?("#{@youtubefolder}#{name}.#{ending}")
                  @songlist << name.split("/")[-1] + ".#{ending}"
                else
                  # Mixin tags and recode it to mp3 (vbr 190kBit)
                  system ("nice -n20 #{@consoleaddition} ffmpeg -i \"#{@tempyoutubefolder}#{name}.#{ending}\" -codec:a libmp3lame -qscale:a 2 -metadata title=\"#{name}\" \"#{@youtubefolder}#{name}.mp3\"") if !File.exist?("#{@youtubefolder}#{name}.mp3")
                  @songlist << name.split("/")[-1] + ".mp3"
                end
              end
            end
            end
            */
          }
      }
    return pair;
  }
}
