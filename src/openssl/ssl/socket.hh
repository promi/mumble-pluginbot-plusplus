/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
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

#include <vector>
#include <cstdint>

#include "network/tcp-socket.hh"
#include "openssl/ssl/context.hh"

namespace OpenSSL::SSL
{
  class Socket
  {
  private:
    TCPSocket &m_socket;
    ::SSL *m_ssl;
  public:
    Socket (TCPSocket &socket, Context &context);
    ~Socket ();
    void connect ();
    void close ();
    std::vector<uint8_t> read (size_t len);
    void write (const std::vector<uint8_t> &data);
  };
}
