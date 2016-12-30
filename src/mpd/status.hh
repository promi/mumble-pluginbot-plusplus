/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2016 Phobos (promi) <prometheus@unterderbruecke.de>

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

#include <mpd/client.h>

#include <cassert>
#include <memory>

namespace Mpd
{
  class Status
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    Status (mpd_status *status);
    Status (Status &&other);
    ~Status ();
    void status_feed (const std::pair<std::string, std::string> &pair);
    int volume () const;
    bool repeat () const;
    bool random () const;
    bool single () const;
    bool consume () const;
    uint queue_length () const;
    uint queue_version () const;
    // State state () const;
    uint crossfade () const;
    float mixrampdb () const;
    float mixrampdelay () const;
    int song_pos () const;
    int song_id () const;
    int next_song_pos () const;
    int next_song_id () const;
    uint elapsed_time ();
    uint elapsed_ms ();
    uint total_time ();
    uint kbit_rate ();
    // AudioFormat audio_format ();
    uint update_id ();
    // std::string error ();
  };
}
