/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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

#include "openssl/ssl/socket.hh"

namespace OpenSSL::SSL
{
  Socket::Socket (TCPSocket &socket, Context &context) : m_socket (socket)
  {
    m_ssl = SSL_new (context.data ());
    if (m_ssl == nullptr)
      {
        throw std::runtime_error ("SSL_new () failed");
      }
    if (SSL_set_fd (m_ssl, socket.data ()) != 1)
      {
        throw std::runtime_error ("SSL_set_fd () failed");
      }
  }

  void Socket::connect ()
  {
    m_socket.connect ();
    if (SSL_connect (m_ssl) != 1)
      {
        throw std::runtime_error ("SSL_connect () failed");
      }
  }

  void Socket::close ()
  {
    // TODO: Does it make sense to handle the result here?
    (void) SSL_shutdown (m_ssl);
  }

  std::vector<uint8_t> Socket::read (size_t len)
  {
    std::vector<uint8_t> buf (len, 0);
    size_t bytes_read = SSL_read (m_ssl, buf.data (), buf.size ());
    if (bytes_read <= 0)
      {
        throw std::runtime_error ("SSL_read () failed");
      }
    else
      {
        //std::cout << "SSL_read (" << len << ") returned " << bytes_read << " bytes: " << std::endl;
        //for (const auto &it : buf)
        //  {
        //    std::cout << (int) it << " ";
        //  }
        //std::cout << std::endl;
      }
    return buf;
  }

  void Socket::write (const std::vector<uint8_t> &data)
  {
    if (SSL_write (m_ssl, data.data (), data.size ()) <= 0)
      {
        throw std::runtime_error ("SSL_write () failed");
      }
  }

  Socket::~Socket ()
  {
    SSL_free (m_ssl);
  }
}
