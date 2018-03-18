/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Meister der Bots
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 loscoala
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

#include <cstddef>
#include <cstdint>
#include <list>
#include <vector>
#include "aither/log.hh"
#include "Mumble.pb.h"
#include "mumble/connection.hh"
#include "mumble/codec.hh"

namespace Mumble
{
  class Mumble2Mumble
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    static const int COMPRESSED_SIZE = 960;
    Mumble2Mumble (const Aither::Log &log, Codec codec, Connection &conn,
                   size_t sample_rate, size_t channels, size_t bitrate);
    ~Mumble2Mumble ();
    void process_udp_tunnel (const MumbleProto::UDPTunnel &message);
    std::list<uint32_t> get_users ();
    std::vector<int16_t> get_frame (uint32_t user_id);
    size_t get_size (uint32_t user_id);
    void produce (const std::vector<int16_t> &frame);
    void codec (Codec codec);
  };
}
