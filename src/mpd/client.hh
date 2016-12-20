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

#include <memory>
#include <string>
#include <cstdint>

#include "mpd/idle.hh"
#include "mpd/flag-set.hh"
#include "mpd/status.hh"

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
      mpd_malloc struct mpd_connection*	mpd_connection_new_async (struct mpd_async *async, const char *welcome)
      const struct mpd_settings* mpd_connection_get_settings (const struct mpd_connection *connection)
      void 	mpd_connection_set_keepalive (struct mpd_connection *connection, bool keepalive)
      void 	mpd_connection_set_timeout (struct mpd_connection *connection, unsigned timeout_ms)
      mpd_pure int 	mpd_connection_get_fd (const struct mpd_connection *connection)
      mpd_pure struct mpd_async * 	mpd_connection_get_async (struct mpd_connection *connection)
      mpd_pure enum mpd_server_error 	mpd_connection_get_server_error (const struct mpd_connection *connection)
      mpd_pure unsigned 	mpd_connection_get_server_error_location (const struct mpd_connection *connection)
      mpd_pure int 	mpd_connection_get_system_error (const struct mpd_connection *connection)
      bool 	mpd_connection_clear_error (struct mpd_connection *connection)
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
  };
}
