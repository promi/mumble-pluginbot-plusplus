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
#pragma once

#include <cstdint>
#include <google/protobuf/message.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "aither/log.hh"
#include "mumble/certificate.hh"

namespace Mumble
{
  class Connection
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    Connection (const Aither::Log &log, const std::string &host, uint16_t port,
                const Certificate &cert);
    ~Connection ();
    bool connected () const;
    void connect ();
    void disconnect ();
    std::pair<int, std::shared_ptr<::google::protobuf::Message>> read_message ();
    void send_udp_tunnel_message (const std::vector<uint8_t> &packet);
    void send_message (uint16_t type, const ::google::protobuf::Message &msg);
  };
}

