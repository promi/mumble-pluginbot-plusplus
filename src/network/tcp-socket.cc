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

// #include <stdio.h>
// #include <stdlib.h>
// #include <netdb.h>
// #nclude <netinet/in.h>
// #include <string.h>
// #include <unistd.h>

#include "network/tcp-socket.hh"

#include <stdexcept>

#include <netdb.h>
#include <strings.h>
#include <unistd.h>

TCPSocket::TCPSocket (const std::string &host, uint16_t port)
{
  m_sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (m_sockfd < 0)
    {
      throw std::runtime_error ("ERROR opening socket");
    }

  hostent *server;
  server = gethostbyname (host.c_str ());
  if (server == nullptr)
    {
      throw std::runtime_error ("ERROR, no such host");
    }

  bzero ((char *) &m_server_addr, sizeof (m_server_addr));
  m_server_addr.sin_family = AF_INET;
  bcopy ((char *) server->h_addr, (char *) &m_server_addr.sin_addr.s_addr,
         server->h_length);
  m_server_addr.sin_port = htons (port);
}

TCPSocket::~TCPSocket ()
{
  disconnect ();
}

void TCPSocket::connect ()
{
  if (::connect (m_sockfd, (struct sockaddr*) &m_server_addr,
                 sizeof (m_server_addr)) < 0)
    {
      throw std::runtime_error ("ERROR connecting");
    }
}

void TCPSocket::disconnect ()
{
  ::close (m_sockfd);
}

std::string TCPSocket::read (uint32_t len)
{
  char* buffer = new char[len + 1];
  bzero(buffer, len + 1);
  auto n = ::read (m_sockfd, buffer, len);
  if (n < 0)
    {
      throw std::runtime_error ("ERROR reading from socket");
    }
  std::string s (buffer, buffer + n);
  delete[] buffer;
  return s;
}

void TCPSocket::write (const std::string &data)
{
  if (::write (m_sockfd, data.data (), data.size ()) < 0)
    {
      throw std::runtime_error ("ERROR writing to socket");
    }
}
