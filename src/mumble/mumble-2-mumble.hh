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
#include <string>
#include <queue>
#include <thread>

#include "aither/log.hh"
#include "mumble/Mumble.pb.h"
#include "mumble/connection.hh"
#include "mumble/packet-data-stream.hh"
#include "opus/constants.hh"
#include "opus/decoder.hh"
#include "opus/encoder.hh"
#include "mumble/codec.hh"

namespace Mumble
{
  class Mumble2Mumble
  {
  public:
    static const int COMPRESSED_SIZE = 960;
  private:
    const Aither::Log &m_log;
    PacketDataStream m_pds;
    PacketDataStream m_sendpds;
    Codec m_codec;
    Connection &m_conn;
    int m_sample_rate;
    int m_channels;
    std::mutex m_pds_lock;
    std::map<uint32_t, Opus::Decoder> m_opus_decoders;
    // std::map<uint32_t, Celt::Decoder> m_celt_decoders;
    std::map<uint32_t, std::queue<std::vector<int16_t>>> m_queues;
    Opus::Encoder m_opus_encoder;
    // Celt::Encoder m_celt_encoder;
    std::vector<int16_t> m_rawaudio;
    uint32_t m_seq = 0;
    std::queue<std::vector<uint8_t>> m_plqueue;
    std::thread m_consume;
    // bool m_consume_running;
  public:
    Mumble2Mumble (const Aither::Log &log, Codec codec, Connection &conn,
                   size_t sample_rate, size_t frame_size, size_t channels, size_t bitrate);
    ~Mumble2Mumble ();
    void process_udp_tunnel (const MumbleProto::UDPTunnel &message);
    std::list<uint32_t> getspeakers ();
    std::vector<int16_t> getframe (uint32_t speaker);
    size_t getsize (uint32_t speaker);
    void produce (const std::vector<int16_t> &frame);
    inline void codec (Codec codec)
    {
      m_codec = codec;
    }
    inline void init_encoder (Codec codec)
    {
      m_codec = codec;
    }
  private:
    void consume ();
  };
}
