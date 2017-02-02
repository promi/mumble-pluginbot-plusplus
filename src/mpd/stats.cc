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
#include "mpd/stats.hh"

#include <stdexcept>

namespace Mpd
{
  struct Stats::Impl
  {
    mpd_stats *m_stats;
    Impl (mpd_stats *stats) : m_stats (stats)
    {
    }
  };

  Stats::Stats (mpd_stats *stats) : pimpl (std::make_unique<Impl> (stats))
  {
  }

  Stats::Stats () : pimpl (std::make_unique<Impl> (nullptr))
  {
    if ((pimpl->m_stats = mpd_stats_begin ()) == nullptr)
      {
        throw std::runtime_error ("mpd_stats_begin () failed");
      }
  }

  Stats::~Stats ()
  {
    mpd_stats_free (pimpl->m_stats);
  }

  void Stats::feed (const std::pair<std::string, std::string> &pair)
  {
    mpd_pair p;
    p.name = pair.first.c_str ();
    p.value = pair.second.c_str ();
    mpd_stats_feed (pimpl->m_stats, &p);
  }

  unsigned Stats::number_of_artists ()
  {
    return mpd_stats_get_number_of_artists (pimpl->m_stats);
  }

  unsigned Stats::number_of_albums ()
  {
    return mpd_stats_get_number_of_albums (pimpl->m_stats);
  }

  unsigned Stats::number_of_songs ()
  {
    return mpd_stats_get_number_of_songs (pimpl->m_stats);
  }

  unsigned long Stats::uptime ()
  {
    return mpd_stats_get_uptime (pimpl->m_stats);
  }

  unsigned long Stats::db_update_time ()
  {
    return mpd_stats_get_db_update_time (pimpl->m_stats);
  }

  unsigned long Stats::play_time ()
  {
    return mpd_stats_get_play_time (pimpl->m_stats);
  }

  unsigned long Stats::db_play_time ()
  {
    return mpd_stats_get_db_play_time (pimpl->m_stats);
  }
}
