/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
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
#include <iostream>
#include <sstream>

#include "openssl/ssl/method.hh"
#include "mumble/connection.hh"
#include "util/endian.hh"

namespace Mumble
{
  Connection::Connection (const std::string &host, uint16_t port,
                          const CertManager &cert_manager) : m_host (host),  m_port (port),
    m_cert_manager (cert_manager), m_connected (false)
  {
  }

  void Connection::connect ()
  {
    using namespace OpenSSL::SSL;
    m_context = std::make_unique<Context> (Method::sslv23 ());
    m_context->verify_none ();
    m_context->key (m_cert_manager.key ());
    m_context->cert (m_cert_manager.cert());
    try
      {
        m_tcp_socket = std::make_unique<TCPSocket> (m_host, m_port);
        m_socket = std::make_unique<Socket> (*m_tcp_socket, *m_context);
        m_socket->connect ();
        m_connected = true;
      }
    catch (const std::string &s)
      {
        std::cerr << "Could not connect to " << m_host << ":" << m_port << std::endl;
      }
    catch (...)
      {
        throw;
      }
  }

  void Connection::disconnect ()
  {
    m_connected = false;
    if (m_socket != nullptr)
      {
        m_socket->close ();
      }
  }

  std::pair<int, std::shared_ptr<::google::protobuf::Message>>
      Connection::read_message ()
  {
    std::vector<uint8_t> prefix;
    while (prefix.size () == 0)
      {
        prefix = read_data (6);
      }
    assert (prefix.size () == 6);

    uint16_t type = EndianUtils::value_from_u16be (prefix, 0);
    uint32_t len = EndianUtils::value_from_u32be (prefix, 2);

    auto data = read_data (len);
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

  std::vector<uint8_t> Connection::make_prefix (uint16_t type, uint32_t len)
  {
    std::vector<uint8_t> prefix;
    EndianUtils::add_to_u16be (prefix, type);
    EndianUtils::add_to_u32be (prefix, len);
    return prefix;
  }

  void Connection::send_udp_packet (const std::vector<uint8_t> &packet)
  {
    uint16_t type = Messages::sym_to_type.at (std::type_index (typeid (
                      MumbleProto::UDPTunnel)));
    send_data (type, packet);
  }

  void Connection::send_message (uint16_t type,
                                 const ::google::protobuf::Message &msg)
  {
    std::stringstream os;
    msg.SerializeToOstream (&os);
    std::string os_str = os.str ();
    send_data (type, std::vector<uint8_t> (std::begin (os_str), std::end (os_str)));
  }

  void Connection::send_data (uint16_t type, const std::vector<uint8_t> &data)
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

  std::vector<uint8_t> Connection::read_data (uint32_t len)
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
