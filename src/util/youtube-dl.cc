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
#include "util/youtube-dl.hh"

#include <sstream>

#include "util/shell.hh"
#include "util/string.hh"

namespace Shell = Util::Shell;
namespace StringUtils = Util::String;

namespace Util
{
  struct YoutubeDl::Impl
  {
    const std::string m_executable;
    Impl (const std::string &executable)
      : m_executable (executable)
    {
    }
    std::string exec (const std::string &arguments);
    std::string nice_exec (const std::string &arguments);
  };

  YoutubeDl::YoutubeDl (const std::string &executable)
    : pimpl (std::make_unique<Impl> (executable))
  {
  }

  YoutubeDl::~YoutubeDl ()
  {
  }

  std::string YoutubeDl::Impl::exec (const std::string &arguments)
  {
    return Shell::exec (StringUtils::intercalate ({m_executable, arguments}));
  }

  std::string YoutubeDl::Impl::nice_exec (const std::string &arguments)
  {
    return Shell::nice_exec (StringUtils::intercalate ({m_executable, arguments}));
  }

  std::string YoutubeDl::version () const
  {
    return pimpl->exec ("--version");
  }

  std::vector<std::pair<std::string, std::string>> YoutubeDl::find_songs (
        const std::string &query,
        size_t max_results)
  {
    std::vector<std::pair<std::string, std::string>> v;
    std::stringstream output {pimpl->nice_exec (StringUtils::intercalate (
      {
        "--max-downloads " + std::to_string (max_results),
        "--get-title",
        "--get-id",
        "https://www.youtube.com/results?search_query=" + Shell::squote (query)
      }))
    };
    std::string id;
    for (std::string title; std::getline (output, title); )
      {
        std::getline (output, id);
        v.push_back (std::make_pair (id, title));
      }
    return v;
  }

  std::vector<std::string> YoutubeDl::get_urls (const std::string &uri)
  {
    std::vector<std::string> urls;
    std::stringstream streams {pimpl->exec ("--get-url " + uri)};
    for (std::string stream; std::getline (streams, stream); )
      {
        if (stream.find ("mime=audio/mp4") != std::string::npos)
          {
            urls.push_back (stream);
          }
      }
    return urls;
  }

  std::vector<std::string> YoutubeDl::get_titles (const std::string &uri)
  {
    std::vector<std::string> v;
    std::stringstream output {pimpl->exec (StringUtils::intercalate ({
        "--get-filename",
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

  std::vector<std::string> YoutubeDl::download (const std::string &uri,
      const FileSystem::path &dir)
  {
    std::vector<std::string> v;
    std::stringstream output {pimpl->nice_exec (StringUtils::intercalate({
        "--write-thumbnail",
        "--extract-audio",
        "--audio-format best",
        "--output " + Shell::squote (dir.string () + "/%(title)s.%(ext)s"),
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
}
