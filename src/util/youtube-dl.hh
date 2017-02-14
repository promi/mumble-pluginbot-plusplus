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
#pragma once

#include <experimental/filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Util
{
  class YoutubeDl
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    YoutubeDl (const std::string &executable = "youtube-dl");
    ~YoutubeDl ();
    std::string version () const;
    std::vector<std::pair<std::string, std::string>> find_songs (
          const std::string &query, size_t max_results);
    std::vector<std::string> get_urls (const std::string &uri);
    std::vector<std::string> get_titles (const std::string &uri);
    std::vector<std::string> download (const std::string &uri,
                                       const std::experimental::filesystem::path &dir);
  };
}
