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
#include "mpd/client.hh"

#include <stdexcept>
#include <mpd/client.h>

namespace Mpd
{
  struct Client::Impl
  {
    mpd_connection *connection;
  };

  Client::Client (const std::string& host, uint16_t port, uint timeout_ms)
  {
    pimpl = std::make_unique<Impl> ();
    pimpl->connection = mpd_connection_new (host.size () == 0 ? nullptr :
                                            host.c_str (),
                                            port, timeout_ms);
    if (pimpl->connection == nullptr)
      {
        throw std::runtime_error ("mpd_connection_new () failed");
      }
    if (mpd_connection_get_error (pimpl->connection) != MPD_ERROR_SUCCESS)
      {
        throw std::runtime_error (std::string ("Could not connect to MPD: ") +
                                  mpd_connection_get_error_message (pimpl->connection));
      }
  }

  Client::~Client ()
  {
    mpd_connection_free (pimpl->connection);
  }
}
