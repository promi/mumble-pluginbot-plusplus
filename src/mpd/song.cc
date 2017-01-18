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
#include "mpd/song.hh"

#include <stdexcept>

namespace Mpd
{
  Song::Song (mpd_song *ptr)
    : m_ptr (ptr)
  {
  }

  Song::Song (const std::pair<std::string, std::string> &pair)
  {
    mpd_pair p;
    p.name = pair.first.c_str ();
    p.value = pair.second.c_str ();
    m_ptr = mpd_song_begin (&p);
    if (m_ptr == nullptr)
      {
        throw std::runtime_error ("mpd_song_begin () failed");
      }
  }

  Song::~Song ()
  {
    mpd_song_free (m_ptr);
  }

  std::string Song::uri () const
  {
    auto uri_ptr = mpd_song_get_uri (m_ptr);
    if (uri_ptr == nullptr)
      {
        throw std::runtime_error ("mpd_song_get_uri () failed");
      }
    return std::string (uri_ptr);
  }

  std::unique_ptr<std::string> Song::tag (TagType type, unsigned idx) const
  {
    auto tag_ptr = mpd_song_get_tag (m_ptr, static_cast<mpd_tag_type> (type), idx);
    if (tag_ptr == nullptr)
      {
        return nullptr;
      }
    else
      {
        return std::make_unique<std::string> (tag_ptr);
      }
  }

  unsigned Song::duration () const
  {
    return mpd_song_get_duration (m_ptr);
  }

  unsigned Song::duration_ms () const
  {
    return mpd_song_get_duration_ms (m_ptr);
  }

  unsigned Song::start () const
  {
    return mpd_song_get_start (m_ptr);
  }

  unsigned Song::end () const
  {
    return mpd_song_get_end (m_ptr);
  }

  time_t Song::last_modified () const
  {
    return mpd_song_get_last_modified (m_ptr);
  }

  void Song::pos (unsigned pos)
  {
    mpd_song_set_pos (m_ptr, pos);
  }

  unsigned Song::pos () const
  {
    return mpd_song_get_pos (m_ptr);
  }

  unsigned Song::id () const
  {
    return mpd_song_get_id (m_ptr);
  }

  unsigned Song::prio () const
  {
    return mpd_song_get_prio (m_ptr);
  }

  bool Song::feed (const std::pair<std::string, std::string> &pair)
  {
    mpd_pair p;
    p.name = pair.first.c_str ();
    p.value = pair.second.c_str ();
    return mpd_song_feed (m_ptr, &p);
  }
}
