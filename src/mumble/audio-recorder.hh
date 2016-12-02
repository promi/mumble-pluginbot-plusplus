/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 dafoxia
    Copyright (c) 2016 dafoxia
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
#include <mutex>
#include <map>
#include <queue>
#include <cstddef>
#include <cstdint>

#include "Mumble.pb.h"
#include "mumble/packet-data-stream.hh"
#include "opus/decoder.hh"

namespace Mumble
{
  struct ClientIntf
  {
  };

  class AudioRecorder
  {
  private:
    ClientIntf &m_client;
    size_t m_sample_rate;
    // WaveFile::Format m_wav_format;
    PacketDataStream m_pds;
    std::mutex m_pds_lock;
    std::map<uint32_t, Opus::Decoder> m_opus_decoders;
    // std::map<uint32_t, Celt::Decoder> m_celt_decoders;
    std::map<uint32_t, std::queue<int>> m_queues;
    bool m_recording = false;
  public:
    AudioRecorder (ClientIntf &client, size_t sample_rate);
    inline bool recording () const
    {
      return m_recording;
    }
    void stream_portaudio ();
    void start (const std::string &file);
    void stop ();
  private:
    void process_udp_tunnel (const MumbleProto::UDPTunnel &message);
    void write_audio ();
    void portaudio ();
  };
}
