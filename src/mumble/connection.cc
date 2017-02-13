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
#include <iostream>
#include <mutex>
#include <sstream>

#include "Mumble.pb.h"
#include "mumble/connection.hh"
#include "mumble/messages.hh"
#include "network/tcp-socket.hh"
#include "openssl/ssl/context.hh"
#include "openssl/ssl/method.hh"
#include "openssl/ssl/socket.hh"
#include "util/endian.hh"

namespace Mumble
{
  struct Connection::Impl
  {
    const Aither::Log &m_log;
    std::string m_host;
    uint16_t m_port;
    const Certificate &m_cert;
    bool m_connected = false;
    std::mutex m_write_lock;
    std::unique_ptr<TCPSocket> m_tcp_socket;
    std::unique_ptr<OpenSSL::SSL::Context> m_context;
    std::unique_ptr<OpenSSL::SSL::Socket> m_socket;
    Impl (const Aither::Log &log, const std::string &host, uint16_t port,
          const Certificate &cert);
    void connect ();
    void disconnect ();
    void send_data (uint16_t type, const std::vector<uint8_t> &data);
    std::vector<uint8_t> read_data (uint32_t len);
    static std::vector<uint8_t> make_prefix (uint16_t type, uint32_t len);
  };

  Connection::Impl::Impl (const Aither::Log &log, const std::string &host,
                          uint16_t port, const Certificate &cert)
    : m_log (log), m_host (host), m_port (port), m_cert (cert)
  {
    (void) m_log;
  }

  Connection::Connection (const Aither::Log &log, const std::string &host,
                          uint16_t port,
                          const Certificate &cert)
    : pimpl (std::make_unique<Impl> (log, host, port, cert))
  {
  }

  Connection::~Connection ()
  {
  }

  bool Connection::connected () const
  {
    return pimpl->m_connected;
  }

  void Connection::Impl::connect ()
  {
    using namespace OpenSSL::SSL;
    m_context = std::make_unique<Context> (Method::sslv23 ());
    auto &context = *m_context;
    context.verify_none ();
    context.key (*m_cert.key);
    context.cert (*m_cert.cert);
    try
      {
        m_tcp_socket = std::make_unique<TCPSocket> (m_host, m_port);
        m_socket = std::make_unique<Socket> (*m_tcp_socket, *m_context);
        m_socket->connect ();
        m_connected = true;
      }
    catch (const std::string &s)
      {
        std::cerr << "Could not connect to " << m_host << ":" << m_port << "\n";
      }
    catch (...)
      {
        throw;
      }
  }

  void Connection::connect ()
  {
    pimpl->connect();
  }

  void Connection::Impl::disconnect ()
  {
    m_connected = false;
    if (m_socket != nullptr)
      {
        m_socket->close ();
      }
  }

  void Connection::disconnect ()
  {
    pimpl->disconnect ();
  }

  std::pair<int, std::shared_ptr<::google::protobuf::Message>>
      Connection::read_message ()
  {
    auto prefix = pimpl->read_data (6);
    uint16_t type = EndianUtils::value_from_u16be (prefix, 0);
    uint32_t len = EndianUtils::value_from_u32be (prefix, 2);

    auto data = pimpl->read_data (len);
    auto msg = Messages::msg_from_type (type);
    if (type == Messages::sym_to_type.at (std::type_index (typeid (
                                            MumbleProto::UDPTunnel))))
      {
        // UDP Packet -- No Protobuf
        auto udp_tunnel = dynamic_cast<MumbleProto::UDPTunnel&>(*msg);
        udp_tunnel.set_packet (std::string (std::begin (data), std::end (data)));
      }
    else
      {
        std::stringstream is (std::string (std::begin (data), std::end (data)));
        msg->ParseFromIstream (&is);
      }
    return std::make_pair (type, std::move (msg));
  }

  std::vector<uint8_t> Connection::Impl::make_prefix (uint16_t type, uint32_t len)
  {
    std::vector<uint8_t> prefix;
    EndianUtils::add_to_u16be (prefix, type);
    EndianUtils::add_to_u32be (prefix, len);
    return prefix;
  }

  void Connection::send_udp_tunnel_message (const std::vector<uint8_t> &packet)
  {
    uint16_t type = Messages::sym_to_type.at (std::type_index (typeid (
                      MumbleProto::UDPTunnel)));
    pimpl->send_data (type, packet);
  }

  void Connection::send_message (uint16_t type,
                                 const ::google::protobuf::Message &msg)
  {
    std::stringstream os;
    msg.SerializeToOstream (&os);
    std::string os_str = os.str ();
    pimpl->send_data (type, std::vector<uint8_t> (std::begin (os_str),
                      std::end (os_str)));
  }

  void Connection::Impl::send_data (uint16_t type,
                                    const std::vector<uint8_t> &data)
  {
    uint32_t len = data.size ();
    auto prefix = make_prefix (type, len);
    if (m_connected)
      {
        std::lock_guard <std::mutex> lock (m_write_lock);
        m_socket->write (prefix);
        m_socket->write (data);
      }
  }

  std::vector<uint8_t> Connection::Impl::read_data (uint32_t len)
  {
    if (m_connected)
      {
        return m_socket->read (len);
      }
    else
      {
        return { };
      }
  }
}
