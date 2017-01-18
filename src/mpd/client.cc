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

  FlagSet<Idle> Client::idle ()
  {
    return FlagSet<Idle> (static_cast<Idle> (mpd_run_idle (pimpl->connection)));
  }

  FlagSet<Idle> Client::idle_mask (FlagSet<Idle> mask)
  {
    return FlagSet<Idle> (static_cast<Idle> (mpd_run_idle_mask (pimpl->connection,
                          static_cast<mpd_idle> (mask.to_enum ()))));
  }

  FlagSet<Idle> Client::noidle ()
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

  Status Client::status ()
  {
    mpd_status *status = mpd_run_status (pimpl->connection);
    if (status == nullptr)
      {
        auto msg_ptr = mpd_connection_get_error_message (pimpl->connection);
        throw std::runtime_error ("mpd_run_status () failed: " + std::string (msg_ptr));
      }
    Status s {status};
    return s;
  }

  Song Client::recv_song ()
  {
    auto song_ptr = mpd_recv_song (pimpl->connection);
    if (song_ptr == nullptr)
      {
        throw std::runtime_error ("mpd_recv_song () failed");
      }
    Song s {song_ptr};
    return s;
  }

  void Client::send_current_song ()
  {
    if (!mpd_send_current_song (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_current_song () failed");
      }
  }

  Song Client::current_song ()
  {
    auto song_ptr = mpd_run_current_song (pimpl->connection);
    if (song_ptr == nullptr)
      {
        throw std::runtime_error ("mpd_run_current_song () failed");
      }
    Song s {song_ptr};
    return s;
  }

  void Client::send_play ()
  {
    if (!mpd_send_play (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_play () failed");
      }
  }

  void Client::play ()
  {
    if (!mpd_run_play (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_run_play () failed");
      }
  }

  void Client::send_play_pos (unsigned song_pos)
  {
    if (!mpd_send_play_pos (pimpl->connection, song_pos))
      {
        throw std::runtime_error ("mpd_send_play_pos () failed");
      }
  }

  void Client::play_pos (unsigned song_pos)
  {
    if (!mpd_run_play_pos (pimpl->connection, song_pos))
      {
        throw std::runtime_error ("mpd_send_run_play_pos () failed");
      }
  }

  void Client::send_play_id (unsigned id)
  {
    if (!mpd_send_play_id (pimpl->connection, id))
      {
        throw std::runtime_error ("mpd_send_play_id () failed");
      }
  }

  void Client::play_id (unsigned song_id)
  {
    if (!mpd_run_play_id (pimpl->connection, song_id))
      {
        throw std::runtime_error ("mpd_send_run_play_id () failed");
      }
  }

  void Client::send_stop ()
  {
    if (!mpd_send_stop (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_stop () failed");
      }
  }

  void Client::stop ()
  {
    if (!mpd_run_stop (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_run_stop () failed");
      }
  }

  void Client::send_toggle_pause ()
  {
    if (!mpd_send_toggle_pause (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_toggle_pause () failed");
      }
  }

  void Client::toggle_pause ()
  {
    if (!mpd_run_toggle_pause (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_run_toggle_pause () failed");
      }
  }

  void Client::send_pause (bool mode)
  {
    if (!mpd_send_pause (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_pause () failed");
      }
  }

  void Client::pause (bool mode)
  {
    if (!mpd_run_pause (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_run_pause () failed");
      }
  }

  void Client::send_next ()
  {
    if (!mpd_send_next (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_next () failed");
      }
  }

  void Client::next ()
  {
    if (!mpd_run_next (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_run_next () failed");
      }
  }

  void Client::send_previous ()
  {
    if (!mpd_send_previous (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_previous () failed");
      }
  }

  void Client::previous ()
  {
    if (!mpd_run_previous (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_run_previous () failed");
      }
  }

  void Client::send_seek_pos (unsigned song_pos, unsigned t)
  {
    if (!mpd_send_seek_pos (pimpl->connection, song_pos, t))
      {
        throw std::runtime_error ("mpd_send_seek_pos () failed");
      }
  }

  void Client::seek_pos (unsigned song_pos, unsigned t)
  {
    if (!mpd_run_seek_pos (pimpl->connection, song_pos, t))
      {
        throw std::runtime_error ("mpd_send_run_seek_pos () failed");
      }
  }

  void Client::send_seek_id (unsigned id, unsigned t)
  {
    if (!mpd_send_seek_id (pimpl->connection, id, t))
      {
        throw std::runtime_error ("mpd_send_seek_id () failed");
      }
  }

  void Client::seek_id (unsigned song_id, unsigned t)
  {
    if (!mpd_run_seek_id (pimpl->connection, song_id, t))
      {
        throw std::runtime_error ("mpd_send_run_seek_id () failed");
      }
  }

  void Client::send_repeat (bool mode)
  {
    if (!mpd_send_repeat (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_repeat () failed");
      }
  }

  void Client::repeat (bool mode)
  {
    if (!mpd_run_repeat (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_run_repeat () failed");
      }
  }

  void Client::send_random (bool mode)
  {
    if (!mpd_send_random (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_random () failed");
      }
  }

  void Client::random (bool mode)
  {
    if (!mpd_run_random (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_run_random () failed");
      }
  }

  void Client::send_single (bool mode)
  {
    if (!mpd_send_single (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_single () failed");
      }
  }

  void Client::single (bool mode)
  {
    if (!mpd_run_single (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_run_single () failed");
      }
  }

  void Client::send_consume (bool mode)
  {
    if (!mpd_send_consume (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_consume () failed");
      }
  }

  void Client::consume (bool mode)
  {
    if (!mpd_run_consume (pimpl->connection, mode))
      {
        throw std::runtime_error ("mpd_send_run_consume () failed");
      }
  }

  void Client::send_crossfade (unsigned seconds)
  {
    if (!mpd_send_crossfade (pimpl->connection, seconds))
      {
        throw std::runtime_error ("mpd_send_crossfade () failed");
      }
  }

  void Client::crossfade (unsigned seconds)
  {
    if (!mpd_run_crossfade (pimpl->connection, seconds))
      {
        throw std::runtime_error ("mpd_send_run_crossfade () failed");
      }
  }

  void Client::send_mixrampdb (float db)
  {
    if (!mpd_send_mixrampdb (pimpl->connection, db))
      {
        throw std::runtime_error ("mpd_send_mixrampdb () failed");
      }
  }

  void Client::mixrampdb (float db)
  {
    if (!mpd_run_mixrampdb (pimpl->connection, db))
      {
        throw std::runtime_error ("mpd_send_run_mixrampdb () failed");
      }
  }

  void Client::send_mixrampdelay (float seconds)
  {
    if (!mpd_send_mixrampdelay (pimpl->connection, seconds))
      {
        throw std::runtime_error ("mpd_send_mixrampdelay () failed");
      }
  }

  void Client::mixrampdelay (float seconds)
  {
    if (!mpd_run_mixrampdelay (pimpl->connection, seconds))
      {
        throw std::runtime_error ("mpd_send_run_mixrampdelay () failed");
      }
  }

  void Client::send_clearerror ()
  {
    if (!mpd_send_clearerror (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_clearerror () failed");
      }
  }

  void Client::clearerror ()
  {
    if (!mpd_run_clearerror (pimpl->connection))
      {
        throw std::runtime_error ("mpd_send_run_clearerror () failed");
      }
  }

  void Client::send_volume (unsigned volume)
  {
    if (!mpd_send_set_volume (pimpl->connection, volume))
      {
        throw std::runtime_error ("mpd_send_set_volume () failed");
      }
  }

  void Client::volume (unsigned volume)
  {
    if (!mpd_run_set_volume (pimpl->connection, volume))
      {
        throw std::runtime_error ("mpd_send_run_set_volume () failed");
      }
  }

  void Client::send_change_volume (int relative_volume)
  {
    if (!mpd_send_change_volume (pimpl->connection, relative_volume))
      {
        throw std::runtime_error ("mpd_send_change_volume () failed");
      }
  }

  void Client::change_volume (int relative_volume)
  {
    if (!mpd_run_change_volume (pimpl->connection, relative_volume))
      {
        throw std::runtime_error ("mpd_send_run_change_volume () failed");
      }
  }
}
