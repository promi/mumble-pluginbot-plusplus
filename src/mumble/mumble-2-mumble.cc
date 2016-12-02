/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Meister der Bots
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 loscoala
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
#include <algorithm>
#include <iostream>
#include <chrono>

#include "mumble/mumble-2-mumble.hh"
#include "mumble/udp-packet.hh"

namespace Mumble
{
  Mumble2Mumble::Mumble2Mumble (Codec codec, Connection &conn,
                                size_t sample_rate,
                                size_t frame_size, size_t channels, size_t bitrate):
    m_codec (codec), m_conn (conn), m_sample_rate (sample_rate),
    m_channels (channels),
    m_opus_encoder (static_cast<Opus::SampleRate> (sample_rate), sample_rate / 100,
                    static_cast<Opus::Channels> (channels), COMPRESSED_SIZE)
/*
m_celt_encoder (sample_rate, sample_rate / 100, channels, [bitrate / 6800, 127].min)
*/
  {
    // TODO: Add other allowed values for opus
    assert (sample_rate == 48'000 /*'*/);
    assert (channels == 1 || channels == 2);
    (void) frame_size;
    // CBR
    m_opus_encoder.vbr (false);
    m_opus_encoder.bitrate (bitrate);
    m_opus_encoder.signal (Opus::Signal::music);
    //m_celt_encoder.vbr_rate = bitrate
    //m_celt_encoder.prediction_request = 0
    m_consume = std::thread {[this] ()
    {
      this->consume();
    }
                            };
  }

  Mumble2Mumble::~Mumble2Mumble ()
  {
    // kill_threads
  }

  void Mumble2Mumble::process_udp_tunnel (const MumbleProto::UDPTunnel &message)
  {
    std::lock_guard <std::mutex> lock (m_pds_lock);
    auto packetstr = message.packet ();
    std::vector<uint8_t> packet_data (packetstr.begin (), packetstr.end ());

    UDPPacket packet;
    packet.data (m_pds, packet_data);

    auto opus_emplaced = m_opus_decoders.emplace (packet.source_session_id,
                         Opus::Decoder (m_sample_rate, m_sample_rate / 100, m_channels));
    auto opus_decoder = opus_emplaced.first->second;
    auto queue_emplaced = m_queues.emplace (packet.source_session_id,
                                            std::queue<std::vector<int16_t>>());
    auto queue = queue_emplaced.first->second;
    // h[k] = Celt::Decoder.new sample_rate, sample_rate / 100, channels
    if (queue.size () <= 200)
      {
        queue.push (opus_decoder.decode (packet.payload));
      }
  }

  std::list<uint32_t> Mumble2Mumble::getspeakers ()
  {
    std::list<uint32_t> list;
    for (const auto& pair : m_queues)
      {
        list.push_back (pair.first);
      }
    return list;
  }

  std::vector<int16_t> Mumble2Mumble::getframe (uint32_t speaker)
  {
    auto queue = m_queues.at (speaker);
    auto frame = queue.front ();
    queue.pop ();
    return frame;
  }

  size_t Mumble2Mumble::getsize (uint32_t speaker)
  {
    return m_queues.at (speaker).size ();
  }

  void Mumble2Mumble::produce (const std::vector<int16_t> &frame)
  {
    std::copy (std::begin (frame), std::end (frame),
               std::back_inserter (m_rawaudio));
  }

  void Mumble2Mumble::consume ()
  {
    auto num_frames = 0;
    switch (m_codec)
      {
      case Codec::opus:
        while (m_rawaudio.size () >= m_opus_encoder.frame_size () * 2)
          {
            num_frames++;
            std::vector <int16_t> part;
            std::move (std::begin (m_rawaudio),
                       std::begin (m_rawaudio) + m_opus_encoder.frame_size () * 2,
                       std::back_inserter (part));
            m_plqueue.push (m_opus_encoder.encode (part));
          }
      /*
      when Codec::alpha
          while @rawaudio.size >= ( @celt_encoder.frame_size * 2 )
      	num_frames =+1
      	part = @rawaudio.slice!( 0, (@celt_encoder.frame_size * 2 ) )
      	@plqueue << @celt_encoder.encode(part)
          end
      when Codec::beta
          while @rawaudio.size >= ( @celt_encoder.frame_size * 2 )
      	num_frames =+1
      	part = @rawaudio.slice!( 0, (@celt_encoder.frame_size * 2 ) )
      	@plqueue << @celt_encoder.encode(part)
          end
      */
      default:
        throw std::string ("m2m invalid codec");
      }
    if (m_plqueue.size () > 0)
      {
        m_seq++;

        auto frame = m_plqueue.front ();
        m_plqueue.pop ();
        try
          {
            UDPPacket packet;
            packet.type = m_codec;
            packet.target = 0;
            packet.sequence_number = m_seq;
            packet.payload = frame;
            m_conn.send_udp_packet (packet.data (m_sendpds));
          }
        catch (...)
          {
            std::cout << "could not write (fatal!)" << std::endl;
          }
      }
    else
      {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for (2ms);
      }
  }
}
