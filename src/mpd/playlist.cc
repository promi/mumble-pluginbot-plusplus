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
#include "mpd/playlist.hh"

namespace Mpd
{
  struct Playlist::Impl
  {
    mpd_playlist *m_playlist;
    Impl (mpd_playlist *playlist) : m_playlist (playlist)
    {
    }
  };

  Playlist::Playlist (mpd_playlist *playlist) : pimpl (std::make_unique<Impl>
        (playlist))
  {
  }

  Playlist::Playlist (const std::pair<std::string, std::string> &pair) : pimpl (
      nullptr)
  {
    mpd_pair p;
    p.name = pair.first.c_str ();
    p.value = pair.second.c_str ();
    if ((pimpl->m_playlist = mpd_playlist_begin (&p)) == nullptr)
      {
        throw std::runtime_error ("mpd_playlist_begin () failed");
      }
  }

  Playlist::Playlist (const Playlist &other) : pimpl (nullptr)
  {
    if ((pimpl->m_playlist = mpd_playlist_dup (other.pimpl->m_playlist)) == nullptr)
      {
        throw std::runtime_error ("mpd_playlist_dup () failed");
      }
  }

  Playlist::Playlist (Playlist &&other) : Playlist (other.pimpl->m_playlist)
  {
    other.pimpl->m_playlist = nullptr;
  }

  Playlist::~Playlist ()
  {
    mpd_playlist_free (pimpl->m_playlist);
  }

  std::string Playlist::path ()
  {
    auto path_ptr = mpd_playlist_get_path (pimpl->m_playlist);
    if (path_ptr == nullptr)
      {
        throw std::runtime_error ("mpd_playlist_get_path () failed");
      }
    return std::string {path_ptr};
  }

  time_t Playlist::last_modified ()
  {
    return mpd_playlist_get_last_modified (pimpl->m_playlist);
  }

  bool Playlist::feed (const std::pair<std::string, std::string> &pair)
  {
    mpd_pair p;
    p.name = pair.first.c_str ();
    p.value = pair.second.c_str ();
    return mpd_playlist_feed (pimpl->m_playlist, &p);
  }
}
