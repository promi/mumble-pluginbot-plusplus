/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2016 Phobos (promi) <prometheus@unterderbruecke.de>
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
#include "mpd/status.hh"

namespace Mpd
{
  struct Status::Impl
  {
    mpd_status *status;
  };

  Status::Status (mpd_status *status) : pimpl (std::make_unique<Impl> ())
  {
    assert(status != nullptr);
    pimpl->status = status;
  }

  Status::Status (Status &&other)
  {
    pimpl->status = other.pimpl->status;
    other.pimpl->status = nullptr;
  }

  Status::~Status ()
  {
    mpd_status_free (pimpl->status);
  }

  void Status::status_feed (const std::pair<std::string, std::string> &pair)
  {
    mpd_pair p;
    p.name = pair.first.c_str ();
    p.value = pair.second.c_str ();
    mpd_status_feed (pimpl->status, &p);
  }

  int Status::volume () const
  {
    return mpd_status_get_volume (pimpl->status);
  }

  bool Status::repeat () const
  {
    return mpd_status_get_repeat (pimpl->status);
  }

  bool Status::random () const
  {
    return mpd_status_get_random (pimpl->status);
  }

  bool Status::single () const
  {
    return mpd_status_get_single (pimpl->status);
  }

  bool Status::consume () const
  {
    return mpd_status_get_consume (pimpl->status);
  }

  uint Status::queue_length () const
  {
    return mpd_status_get_queue_length (pimpl->status);
  }

  uint Status::queue_version () const
  {
    return mpd_status_get_queue_version (pimpl->status);
  }

  State Status::state () const
  {
    return static_cast<State> (mpd_status_get_state (pimpl->status));
  }

  uint Status::crossfade () const
  {
    return mpd_status_get_crossfade (pimpl->status);
  }

  float Status::mixrampdb () const
  {
    return mpd_status_get_mixrampdb (pimpl->status);
  }

  float Status::mixrampdelay () const
  {
    return mpd_status_get_mixrampdelay (pimpl->status);
  }

  int Status::song_pos () const
  {
    return mpd_status_get_song_pos (pimpl->status);
  }

  int Status::song_id () const
  {
    return mpd_status_get_song_id (pimpl->status);
  }

  int Status::next_song_pos () const
  {
    return mpd_status_get_next_song_pos (pimpl->status);
  }

  int Status::next_song_id () const
  {
    return mpd_status_get_next_song_id (pimpl->status);
  }

  uint Status::elapsed_time () const
  {
    return mpd_status_get_elapsed_time (pimpl->status);
  }

  uint Status::elapsed_ms () const
  {
    return mpd_status_get_elapsed_ms (pimpl->status);
  }

  uint Status::total_time () const
  {
    return mpd_status_get_total_time (pimpl->status);
  }

  uint Status::kbit_rate () const
  {
    return mpd_status_get_kbit_rate (pimpl->status);
  }

  // AudioFormat Status::audio_format () const

  uint Status::update_id () const
  {
    return mpd_status_get_update_id (pimpl->status);
  }

  // std::string Status::error () const

}
