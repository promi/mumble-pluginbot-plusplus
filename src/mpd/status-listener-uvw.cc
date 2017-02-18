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
#include "mpd/status-listener.hh"

#include "uvw.hpp"

namespace Mpd
{
  struct StatusListener::Impl
  {
    std::string ip;
    uint16_t port;
    std::function<void (const FlagSet<Idle> &)> status_cb;
    std::shared_ptr <uvw::Loop> loop;
    std::shared_ptr <uvw::AsyncHandle> stop;
    StatusListener::State state = StatusListener::State::NotStarted;

    inline Impl (const std::string &ip, uint16_t port,
                 std::function<void (const FlagSet<Idle> &)> status_cb)
      : ip (ip), port (port), status_cb (status_cb)
    {
    }
    void send_command (uvw::TcpHandle &tcp, const std::string &command);
    void process_line (uvw::TcpHandle &tcp, std::string &line);
    void process_data (uvw::TcpHandle &tcp, std::string &buf);
  };

  StatusListener::StatusListener (const std::string &ip, uint16_t port,
                                  std::function<void (const FlagSet<Idle> &)> status_cb)
    : pimpl (new Impl (ip, port, status_cb))
  {
    pimpl->loop = uvw::Loop::getDefault ();
    if (pimpl->loop == nullptr)
      {
        throw std::runtime_error ("loop init failed");
      }
    pimpl->stop = pimpl->loop->resource<uvw::AsyncHandle>();
    if (pimpl->stop == nullptr)
      {
        throw std::runtime_error ("stop init failed");
      }
  }

  StatusListener::~StatusListener ()
  {

  }

  void StatusListener::stop ()
  {
    pimpl->stop->send ();
  }

  void StatusListener::Impl::send_command (uvw::TcpHandle &tcp,
      const std::string &command)
  {
    const std::string &line = command + "\n";
    auto line_size = line.size ();
    std::unique_ptr<char[]> ptr {new char[line_size]};
    line.copy (ptr.get (), line_size);
    tcp.write (std::move (ptr), line_size);
  }

  void StatusListener::Impl::process_line (uvw::TcpHandle &tcp, std::string &line)
  {
    const std::string ok = "OK";
    const std::string changed = "changed: ";

    if (!line.compare (0, ok.size (), ok))
      {
        // Two forms are possible:
        // "OK MPD v..." -> Connection established, mpd version is ...
        // "OK" all by itself -> Response to a prior "idle" command
        //
        send_command (tcp, "idle");
      }
    else if (!line.compare (0, changed.size (), changed))
      {
        const std::string idle_str = line.substr (changed.size ());
        Idle idle;
        if (idle_str == "database")
          {
            idle = Idle::Database;
          }
        else if (idle_str == "update")
          {
            idle = Idle::Update;
          }
        else if (idle_str == "stored_playlist")
          {
            idle = Idle::StoredPlaylist;
          }
        else if (idle_str == "playlist")
          {
            idle = Idle::Playlist;
          }
        else if (idle_str == "player")
          {
            idle = Idle::Player;
          }
        else if (idle_str == "mixer")
          {
            idle = Idle::Mixer;
          }
        else if (idle_str == "output")
          {
            idle = Idle::Output;
          }
        else if (idle_str == "options")
          {
            idle = Idle::Options;
          }
        else if (idle_str == "sticker")
          {
            idle = Idle::Sticker;
          }
        else if (idle_str == "subscription")
          {
            idle = Idle::Subscription;
          }
        else if (idle_str == "message")
          {
            idle = Idle::Message;
          }
        else
          {
            // Unknown changed string (i.e. a future extension)
            return;
          }
        // TODO: emit signal with the changed status as Mpd::Idle
        // Maybe first combine to flag set when OK is received
        status_cb (FlagSet<Idle> {idle});
      }
    else
      {
        // Unexpected data
        // TODO: Set error status and stop loop?
      }
  }

  void StatusListener::Impl::process_data (uvw::TcpHandle &tcp, std::string &buf)
  {
    std::string::size_type pos;
    while ((pos = buf.find ('\n')) != std::string::npos)
      {
        std::string line = buf.substr (0, pos);
        buf.erase (0, pos + 1);
        process_line (tcp, line);
      }
  }

  void StatusListener::run ()
  {
    pimpl->stop->once<uvw::AsyncEvent> ([this] (const uvw::AsyncEvent &,
                                        uvw::AsyncHandle &) mutable
    {
      pimpl->state = State::Stopped;
      pimpl->loop->stop ();
    });

    auto tcp = pimpl->loop->resource<uvw::TcpHandle> ();
    if (tcp == nullptr)
      {
        throw std::runtime_error ("tcp init failed");
      }

    tcp->once<uvw::ConnectEvent> ([] (const uvw::ConnectEvent &,
                                      uvw::TcpHandle &tcp) mutable
    {
      tcp.read ();
    });

    tcp->once<uvw::ErrorEvent> ([this] (const uvw::ErrorEvent &,
                                        uvw::TcpHandle &tcp) mutable
    {
      tcp.close ();
      if (pimpl->state != State::Stopped)
        {
          pimpl->state = State::UnableToConnect;
        }
      pimpl->loop->stop ();
    });

    tcp->once<uvw::EndEvent> ([this] (const uvw::EndEvent &,
                                      uvw::TcpHandle &tcp) mutable
    {
      tcp.close ();
      if (pimpl->state != State::Stopped)
        {
          pimpl->state = State::Disconnected;
        }
      pimpl->loop->stop ();
    });

    std::string buf;
    tcp->on<uvw::DataEvent> ([this, &buf] (const uvw::DataEvent &event,
                                           uvw::TcpHandle &tcp) mutable
    {
      std::string data {event.data.get (), event.length};
      buf += data;
      pimpl->process_data (tcp, buf);
    });

    tcp->connect (pimpl->ip, pimpl->port);
    pimpl->state = State::Started;
    pimpl->loop->run<uvw::Loop::Mode::DEFAULT> ();
  }

  StatusListener::State StatusListener::state ()
  {
    return pimpl->state;
  }
}

