/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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

#include <memory>
#include <string>
#include <mpd/client.h>

namespace Mpd
{
  class Playlist
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    Playlist (mpd_playlist *playlist);
    Playlist (const std::pair<std::string, std::string> &pair);
    Playlist (const Playlist &other);
    Playlist (Playlist &&other);
    ~Playlist ();
    std::string path ();
    time_t last_modified ();
    bool feed (const std::pair<std::string, std::string> &pair);
  };
}
