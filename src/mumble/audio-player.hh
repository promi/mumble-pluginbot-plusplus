/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 loscoala
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

#include <chrono>
#include <thread>
#include <memory>
#include <vector>
#include <queue>

#include "io/sample-reader.hh"
#include "io/raw-s16le-file-sample-reader.hh"
#include "mumble/connection.hh"
#include "io/wavefile-format.hh"
#include "io/wavefile-reader.hh"
#include "opus/encoder.hh"
#include "mumble/codec.hh"
#include "mumble/packet-data-stream.hh"

namespace Mumble
{
  class AudioPlayer
  {
  private:
    int m_volume = 100;
    bool m_playing = false;
    Connection &m_conn;
    // WaveFile::Format m_wav_format;
    Codec m_codec;
    size_t m_bitrate;
    size_t m_sample_rate;
    size_t m_framesize;
    std::unique_ptr<SampleReader> m_file;
    std::thread m_bounded_produce_thread;
    std::unique_ptr<Opus::Encoder> m_encoder;
    uint32_t m_seq = 0;
    std::queue<std::vector<uint8_t>> m_queue;
    PacketDataStream m_pds;
  public:
    static const int COMPRESSED_SIZE = 48;
    AudioPlayer (Codec codec, Connection &connection, size_t sample_rate,
                 size_t bitrate);
    ~AudioPlayer();
    void codec (Codec val);
    inline void volume (int vol)
    {
      m_volume = vol;
    }
    inline int volume () const
    {
      return m_volume;
    }
    inline bool playing () const
    {
      return m_playing;
    }
    void play_file (const std::string &file);
    void stream_named_pipe (const std::string &pipe);
    bool stream_portaudio ();
    void stop ();
    void bitrate (size_t bitrate);
    inline int bitrate () const
    {
      return m_bitrate;
    }
    void framelength (const std::chrono::milliseconds ms);
    inline std::chrono::milliseconds framelength ()
    {
      return std::chrono::milliseconds (m_framesize / COMPRESSED_SIZE);
    }
  private:
    void create_encoder (size_t sample_rate, size_t bitrate);
    std::vector<int16_t> change_volume (const std::vector<int16_t> &samples);
    void bounded_produce ();
    void produce ();
    void portaudio ();
    void encode_samples (const std::vector<int16_t> &samples);
    void consume ();
  };
}

