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

#include <utility>
#include <string>
#include <memory>
#include <mpd/client.h>

#include "mpd/tag-type.hh"

namespace Mpd
{
  class Song
  {
  private:
    mpd_song *m_ptr;
  public:
    Song (mpd_song *ptr);
    Song (const std::pair<std::string, std::string> &pair);
    ~Song ();
    std::string uri () const;
    std::unique_ptr<std::string> tag (TagType type, unsigned idx) const;
    unsigned duration () const;
    unsigned duration_ms () const;
    unsigned start () const;
    unsigned end () const;
    time_t last_modified () const;
    void pos (unsigned pos);
    unsigned pos () const;
    unsigned id () const;
    unsigned prio () const;
    bool feed (const std::pair<std::string, std::string> &pair);
  };
}
