/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2012 Matthew Perry (mattvperry)
    Copyright (c) 2012 Iván Eixarch (joker-x)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 dafoxia
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

#include <string>
#include <utility>
#include <memory>
#include <mutex>
#include <vector>
#include <google/protobuf/message.h>

#include "mumble/cert-manager.hh"
#include "network/tcp-socket.hh"
#include "openssl/ssl/context.hh"
#include "openssl/ssl/socket.hh"
#include "mumble/messages.hh"
#include "Mumble.pb.h"

namespace Mumble
{
  class Connection
  {
  public:
    Connection (const std::string &host, uint16_t port,
                const CertManager &cert_manager);
    void connect ();
    void disconnect ();
    std::pair<int, std::shared_ptr<::google::protobuf::Message>> read_message ();
    void send_udp_packet (const std::vector<uint8_t> &packet);
    void send_message (uint16_t type, const ::google::protobuf::Message &msg);
    inline auto connected () const
    {
      return m_connected;
    }
  private:
    std::string m_host;
    uint16_t m_port;
    const CertManager &m_cert_manager;
    bool m_connected;
    std::mutex m_write_lock;
    std::unique_ptr<TCPSocket> m_tcp_socket;
    std::unique_ptr<OpenSSL::SSL::Context> m_context;
    std::unique_ptr<OpenSSL::SSL::Socket> m_socket;
    void send_data (uint16_t type, const std::vector<uint8_t> &data);
    std::vector<uint8_t> read_data (uint32_t len);
    static std::vector<uint8_t> make_prefix (uint16_t type, uint32_t len);
  };
}

