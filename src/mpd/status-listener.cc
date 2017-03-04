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

#include <chrono>
#include <thread>

#include "network/tcp-socket.hh"

namespace Mpd
{
  struct StatusListener::Impl
  {
    bool m_stop_signaled = false;
    State m_state = State::NotStarted;
    TCPSocket m_socket;
    std::function<void (const FlagSet<Idle> &)> m_status_cb;

    Impl (const std::string &ip, uint16_t port,
          std::function<void (const FlagSet<Idle> &)> status_cb)
      : m_socket (ip, port), m_status_cb (status_cb)
    {
    }
  };

  StatusListener::StatusListener (const std::string &ip, uint16_t port,
                                  std::function<void (const FlagSet<Idle> &)> status_cb)
    : pimpl (std::make_unique<Impl> (ip, port, status_cb))
  {
  }

  StatusListener::~StatusListener ()
  {
  }

  void StatusListener::stop ()
  {
    pimpl->m_stop_signaled = true;
  }

  void StatusListener::run ()
  {
    using namespace std::chrono_literals;

    pimpl->m_state = State::Started;
    try
      {
        pimpl->m_socket.connect ();
      }
    catch (const std::exception &e)
      {
        pimpl->m_state = State::UnableToConnect;
        return;
      }
    while (!pimpl->m_stop_signaled)
      {
        // TODO: Actually send the "idle" command to MPD and parse the answer
        // This is already implemented in status-listener-uvw.cc (but depdends on libuv)
        // WORKAROUND: Just always return Idle::Player every 2 seconds
        pimpl->m_status_cb (FlagSet<Idle> (Idle::Player));
        std::this_thread::sleep_for (2000ms);
      }
    pimpl->m_state = State::Stopped;
  }

  StatusListener::State StatusListener::state ()
  {
    return pimpl->m_state;
  }
}

