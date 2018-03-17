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
#include "mumble/mumble-2-mumble.hh"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

#include "opus/decoder.hh"
#include "opus/encoder.hh"
#include "mumble/udp-packet.hh"

namespace Mumble
{
  struct Mumble2Mumble::Impl
  {
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
    Impl (const Aither::Log &log, Codec codec, Connection &conn, size_t sample_rate,
          size_t channels, size_t bitrate)
      : m_log (log), m_codec (codec), m_conn (conn), m_sample_rate (sample_rate),
        m_channels (channels), m_opus_encoder (m_log,
            static_cast<Opus::SampleRate> (sample_rate), sample_rate / 100,
            static_cast<Opus::Channels> (channels), COMPRESSED_SIZE)
    /*
    m_celt_encoder (sample_rate, sample_rate / 100, channels, [bitrate / 6800, 127].min)
    */
    {
      // TODO: Add other allowed values for opus
      assert (sample_rate == 48 * 1000);
      assert (channels == 1 || channels == 2);
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
    void process_udp_tunnel (const MumbleProto::UDPTunnel &message);
    void consume ();
  };

  void Mumble2Mumble::Impl::process_udp_tunnel (const MumbleProto::UDPTunnel
      &message)
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

  Mumble2Mumble::Mumble2Mumble (const Aither::Log &log, Codec codec,
                                Connection &conn,
                                size_t sample_rate,
                                size_t channels, size_t bitrate)
    : pimpl (std::make_unique<Impl> (log, codec, conn, sample_rate, channels,
                                     bitrate))
  {
  }

  Mumble2Mumble::~Mumble2Mumble ()
  {
  }

  void Mumble2Mumble::process_udp_tunnel (const MumbleProto::UDPTunnel &message)
  {
    pimpl->process_udp_tunnel (message);
  }

  std::list<uint32_t> Mumble2Mumble::get_users ()
  {
    std::list<uint32_t> list;
    for (const auto& pair : pimpl->m_queues)
      {
        list.push_back (pair.first);
      }
    return list;
  }

  std::vector<int16_t> Mumble2Mumble::get_frame (uint32_t user_id)
  {
    auto queue = pimpl->m_queues.at (user_id);
    auto frame = queue.front ();
    queue.pop ();
    return frame;
  }

  size_t Mumble2Mumble::get_size (uint32_t user_id)
  {
    return pimpl->m_queues.at (user_id).size ();
  }

  void Mumble2Mumble::produce (const std::vector<int16_t> &frame)
  {
    std::copy (std::begin (frame), std::end (frame),
               std::back_inserter (pimpl->m_rawaudio));
  }

  void Mumble2Mumble::Impl::consume ()
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
        break;
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
        throw std::runtime_error ("m2m invalid codec");
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
            m_conn.send_udp_tunnel_message (packet.data (m_sendpds));
          }
        catch (...)
          {
            std::cout << "could not write (fatal!)\n";
          }
      }
    else
      {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for (2ms);
      }
  }
}
