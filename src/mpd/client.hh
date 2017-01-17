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
#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "mpd/error.hh"
#include "mpd/idle.hh"
#include "mpd/flag-set.hh"
#include "mpd/status.hh"
#include "mpd/song.hh"

namespace Mpd
{
  class Client
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    Client (const std::string& host = "", uint16_t port = 0,
            uint timeout_ms = 0);
    ~Client ();
    // Connection
    /*
      mpd_malloc struct mpd_connection*	mpd_connection_new_async (
      struct mpd_async *async, const char *welcome)
      const struct mpd_settings* mpd_connection_get_settings (
      const struct mpd_connection *connection)
      void 	mpd_connection_set_keepalive (
      struct mpd_connection *connection, bool keepalive)
      void 	mpd_connection_set_timeout (
      struct mpd_connection *connection, unsigned timeout_ms)
      mpd_pure int 	mpd_connection_get_fd (const struct mpd_connection *connection)
      mpd_pure struct mpd_async * 	mpd_connection_get_async (
      struct mpd_connection *connection)
    */
    Error	error ();
    std::string	error_message ();
    /*
      mpd_pure enum mpd_server_error 	mpd_connection_get_server_error (
      const struct mpd_connection *connection)
      mpd_pure unsigned 	mpd_connection_get_server_error_location (
      const struct mpd_connection *connection)
      mpd_pure int 	mpd_connection_get_system_error (
      const struct mpd_connection *connection)
    */
    void clear_error ();
    /*
      mpd_pure const unsigned * 	mpd_connection_get_server_version (const struct mpd_connection *connection)
      mpd_pure int 	mpd_connection_cmp_server_version (const struct mpd_connection *connection, unsigned major, unsigned minor, unsigned patch)
    */
    // Idle
    static std::string idle_name (Idle idle);
    static Idle idle_name_parse (const std::string &name);
    void send_idle ();
    void send_idle_mask (FlagSet<Idle> mask);
    void send_noidle ();
    static Idle idle_parse_pair (std::pair<std::string, std::string> &pair);
    FlagSet<Idle> recv_idle (bool disable_timeout);
    FlagSet<Idle> run_idle ();
    FlagSet<Idle> run_idle_mask (FlagSet<Idle> mask);
    FlagSet<Idle> run_noidle ();
    // Status
    void send_status ();
    Status recv_status ();
    Status run_status ();
    // Song
    Song recv_song ();
    // Player
    void send_current_song ();
    Song run_current_song ();
    void send_play ();
    void run_play ();
    void send_play_pos (unsigned song_pos);
    void run_play_pos (unsigned song_pos);
    void send_play_id (unsigned id);
    void run_play_id (unsigned song_id);
    void send_stop ();
    void run_stop ();
    void send_toggle_pause ();
    void run_toggle_pause ();
    void send_pause (bool mode);
    void run_pause (bool mode);
    void send_next ();
    void run_next ();
    void send_previous ();
    void run_previous ();
    void send_seek_pos (unsigned song_pos, unsigned t);
    void run_seek_pos (unsigned song_pos, unsigned t);
    void send_seek_id (unsigned id, unsigned t);
    void run_seek_id (unsigned song_id, unsigned t);
    void send_repeat (bool mode);
    void run_repeat (bool mode);
    void send_random (bool mode);
    void run_random (bool mode);
    void send_single (bool mode);
    void run_single (bool mode);
    void send_consume (bool mode);
    void run_consume (bool mode);
    void send_crossfade (unsigned seconds);
    void run_crossfade (unsigned seconds);
    void send_mixrampdb (float db);
    void run_mixrampdb (float db);
    void send_mixrampdelay (float seconds);
    void run_mixrampdelay (float seconds);
    void send_clearerror ();
    void run_clearerror ();
    // Mixer
    void send_set_volume (unsigned volume);
    void run_set_volume (unsigned volume);
    void send_change_volume (int relative_volume);
    void run_change_volume (int relative_volume);
  };
}
