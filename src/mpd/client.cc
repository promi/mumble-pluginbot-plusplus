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

  Error	Client::error ()
  {
    return static_cast<Error> (mpd_connection_get_error (pimpl->connection));
  }
  
  std::string	Client::error_message ()
  {
    return mpd_connection_get_error_message (pimpl->connection);
  }
  
  void Client::clear_error ()
  {
    if (!mpd_connection_clear_error (pimpl->connection))
      {
        throw std::runtime_error ("mpd_clear_error () failed");
      }
  }
  
  std::string Client::idle_name (Idle idle)
  {
    auto name = mpd_idle_name (static_cast<mpd_idle> (idle));
    if (name == nullptr)
      {
        throw std::runtime_error ("mpd_idle_name () failed");
      }
    return name;
  }

  Idle Client::idle_name_parse (const std::string &name)
  {
    auto idle = mpd_idle_name_parse (name.c_str ());
    if (idle == 0)
      {
        throw std::runtime_error ("mpd_idle_name_parse () failed");
      }
    return static_cast<Idle> (idle);
  }

  void Client::send_idle ()
  {
    if (!mpd_send_idle (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_idle () failed");
      }
  }

  void Client::send_idle_mask (FlagSet<Idle> mask)
  {
    if (!mpd_send_idle_mask (pimpl->connection,
                             static_cast<mpd_idle> (mask.to_enum ())))
      {
        throw std::runtime_error ("mpd_send_idle_mask () failed");
      }
  }

  void Client::send_noidle ()
  {
    if(!mpd_send_noidle (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_noidle () failed");
      }
  }

  Idle Client::idle_parse_pair (std::pair<std::string, std::string>
                                &pair)
  {
    mpd_pair p;
    p.name = pair.first.c_str ();
    p.value = pair.second.c_str ();
    auto idle = mpd_idle_parse_pair (&p);
    if (idle == 0)
      {
        throw std::runtime_error ("mpd_idle_parse_pair () failed");
      }
    return static_cast<Idle> (idle);
  }

  FlagSet<Idle> Client::recv_idle (bool disable_timeout)
  {
    return FlagSet<Idle> (static_cast<Idle> (mpd_recv_idle (pimpl->connection,
                          disable_timeout)));
  }

  FlagSet<Idle> Client::run_idle ()
  {
    return FlagSet<Idle> (static_cast<Idle> (mpd_run_idle (pimpl->connection)));
  }

  FlagSet<Idle> Client::run_idle_mask (FlagSet<Idle> mask)
  {
    return FlagSet<Idle> (static_cast<Idle> (mpd_run_idle_mask (pimpl->connection,
                          static_cast<mpd_idle> (mask.to_enum ()))));
  }

  FlagSet<Idle> Client::run_noidle ()
  {
    return FlagSet<Idle> (static_cast<Idle> (mpd_run_noidle (pimpl->connection)));
  }

  void Client::send_status ()
  {
    if (!mpd_send_status (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_status () failed");
      }
  }

  Status Client::recv_status ()
  {
    mpd_status *status = mpd_recv_status (pimpl->connection);
    if (status == nullptr)
      {
        throw std::runtime_error ("mpd_recv_status () failed");
      }
    Status s {status};
    return s;
  }

  Status Client::run_status ()
  {
    mpd_status *status = mpd_run_status (pimpl->connection);
    if (status == nullptr)
      {
        throw std::runtime_error ("mpd_run_status () failed: " + std::string (mpd_connection_get_error_message (pimpl->connection)));
      }
    Status s {status};
    return s;
  }
}
