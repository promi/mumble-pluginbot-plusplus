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
#include "util/ffmpeg.hh"

#include <sstream>

#include "util/shell.hh"
#include "util/string.hh"

namespace Shell = Util::Shell;
namespace StringUtils = Util::String;

namespace Util
{
  struct Ffmpeg::Impl
  {
    Impl (const std::string &executable)
      : m_executable (executable)
    {
    }
    const std::string m_executable;
  };

  Ffmpeg::Ffmpeg (const std::string &executable)
    : pimpl (std::make_unique<Impl> (executable))
  {
  }

  Ffmpeg::~Ffmpeg ()
  {
  }

  std::vector<std::string> Ffmpeg::convert_to_mp3 (
    const FileSystem::path &from, const FileSystem::path &to,
    const std::string &title)
  {
    std::vector<std::string> v;
    std::stringstream output {Shell::nice_exec (StringUtils::intercalate ({
        pimpl->m_executable,
        "-n",
        "-i " + Shell::squote (from.string ()),
        "-codec:a libmp3lame",
        "-qscale:a 2",
        "-metadata title=" + Shell::squote (title),
        Shell::squote (to.string ()),
        "2>&1"
      }))
    };
    for (std::string line; std::getline (output, line); )
      {
        v.push_back (line);
      }
    return v;
  }

  std::vector<std::string> Ffmpeg::tag (const FileSystem::path &from,
                                        const FileSystem::path &to, const std::string &title)
  {
    std::vector<std::string> v;
    std::stringstream output {Shell::nice_exec (StringUtils::intercalate (
      {
        pimpl->m_executable,
        "-n",
        "-i " + Shell::squote (from.string ()),
        "-codec:a copy",
        "-metadata title=" + Shell::squote (title),
        Shell::squote (to.string ()),
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
