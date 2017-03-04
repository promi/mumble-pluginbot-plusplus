/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 loscoala
    Copyright (c) 2016 dafoxia
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
#include "mumble/audio-player.hh"
#include "mumble/udp-packet.hh"

#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "io/raw-s16le-file-sample-reader.hh"
#include "io/sample-reader.hh"
#include "io/wavefile-format.hh"
#include "io/wavefile-reader.hh"
#include "mumble/packet-data-stream.hh"
#include "opus/encoder.hh"

namespace Mumble
{
  struct AudioPlayer::Impl
  {
    static const int COMPRESSED_SIZE = 48;
    const Aither::Log &m_log;
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
    Impl (const Aither::Log &log, Codec codec, Connection &connection,
          size_t sample_rate, size_t bitrate);
    void create_encoder (size_t sample_rate, size_t bitrate);
    std::vector<int16_t> change_volume (const std::vector<int16_t> &samples);
    void bounded_produce ();
    void produce ();
    void portaudio ();
    void encode_samples (const std::vector<int16_t> &samples);
    void consume ();
  };

  namespace wav = WaveFile;
  // namespace pa = PortAudio;

  AudioPlayer::Impl::Impl (const Aither::Log &log, Codec codec,
                           Connection &connection, size_t sample_rate, size_t bitrate)
    : m_log (log), m_conn (connection),
  /* m_wav_format (wav::Format (wav::Channels::mono, wav::SampleFormat::pcm_16,
     static_cast<size_t> (sample_rate))),*/
      m_codec (codec), m_bitrate (bitrate), m_sample_rate (sample_rate),
      m_framesize (COMPRESSED_SIZE * 10)
  {
    AITHER_DEBUG("Creating AudioPlayer with codec = " << to_string (codec) <<
                 ", sample_rate = " << sample_rate <<
                 ", bitrate = " << bitrate);
    create_encoder (m_sample_rate, m_bitrate);
  }

  void AudioPlayer::Impl::bounded_produce ()
  {
    // TODO: Check if frame_size is in bytes or in samples
    m_file->each_buffer (m_encoder->frame_size (), [this] (const auto &samples)
    {
      this->encode_samples (samples);
      this->consume ();
      return this->m_playing;
    });
    m_playing = false;
  }

  void AudioPlayer::Impl::produce ()
  {
    // TODO: Check if frame_size is in bytes or in samples
    encode_samples (m_file->read (m_encoder->frame_size ()));
    consume ();
  }

  void AudioPlayer::Impl::portaudio ()
  {
    /*
        begin
          @portaudio.read(@audiobuffer)
          @queue << @encoder.encode_ptr(@audiobuffer.to_ptr)
          consume
        rescue
          sleep 0.2
        end
     */
  }

  void AudioPlayer::Impl::encode_samples (const std::vector<int16_t> &samples)
  {
    if (m_volume == 100)
      {
        m_queue.push (m_encoder->encode (samples));
      }
    else
      {
        m_queue.push (m_encoder->encode (change_volume (samples)));
      }
  }

  void AudioPlayer::Impl::consume (void)
  {
    m_seq %= 1'000'000;
    m_seq++;

    auto frame = m_queue.front ();
    m_queue.pop ();

    UDPPacket packet;
    packet.type = m_codec;
    packet.target = 0;
    packet.sequence_number = m_seq;
    packet.payload = frame;
    m_conn.send_udp_tunnel_message (packet.data (m_pds));
  }

  void AudioPlayer::Impl::create_encoder (size_t sample_rate, size_t bitrate)
  {
    // kill_threads ();
    m_encoder = nullptr;

    if (m_codec == Codec::alpha || m_codec == Codec::beta)
      {
        /*
              m_encoder = Celt::Encoder (sample_rate, sample_rate / 100, 1,
                                         clamp (bitrate / 800, 0, 127));
              m_encoder.vbr_rate (bitrate);
              m_encoder.prediction_request (false);
        */
      }
    else
      {
        // TODO: check other valid sample_rates (see opus-constants.h)
        assert (sample_rate == 48'000 /*'*/);
        m_encoder = std::make_unique<Opus::Encoder>
                    (m_log, static_cast<Opus::SampleRate> (sample_rate), m_framesize,
                     Opus::Channels::mono, 7200);
        m_encoder->bitrate (bitrate);
        // constrainted vbr doesn't work with Opus::Signal::voice
        m_encoder->signal (Opus::Signal::music);
        m_encoder->vbr (true);
        m_encoder->vbr_constraint (true);
        m_encoder->packet_loss_percentage (10);
        m_encoder->dtx (true);
      }
  }

  std::vector<int16_t> AudioPlayer::Impl::change_volume (const
      std::vector<int16_t>
      &samples)
  {
    std::vector<int16_t> v;
    for (const auto &s : samples)
      {
        //std::transform (std::begin (samples), std::end (samples),
        //                std::back_inserter (v),
        //                [this] (int16_t s)
        //{
        //  return s * (m_volume / 100.0);
        //});
        v.push_back (s * (m_volume / 100.0));
      }
    return v;
  }

  AudioPlayer::AudioPlayer (const Aither::Log &log, Codec codec,
                            Connection &connection, size_t sample_rate,
                            size_t bitrate) : pimpl (std::make_unique<Impl> (log, codec, connection,
                                  sample_rate, bitrate))
  {
  }

  AudioPlayer::~AudioPlayer ()
  {
    pimpl->m_bounded_produce_thread.join ();
    // kill_threads ()
  }

  void AudioPlayer::codec (Codec val)
  {
    pimpl->m_codec = val;
    pimpl->create_encoder (pimpl->m_sample_rate, pimpl->m_bitrate);
  }

  void AudioPlayer::play_file (const FileSystem::path &file)
  {
    (void) file;
    throw std::runtime_error ("not fully implemented yet");
    /*
    if (!m_playing)
      {
        m_file = std::make_unique<wav::Reader> (file, m_wav_format);
        m_bounded_produce_thread = std::thread ([this] ()
        {
          bounded_produce ();
        });
        m_playing = true;
      }
    */
  }

  void AudioPlayer::stream_named_pipe (const FileSystem::path &file)
  {
    if (!pimpl->m_playing)
      {
        pimpl->m_file = std::make_unique<RawFileReader> (file);
        pimpl->m_bounded_produce_thread = std::thread ([this] ()
        {
          pimpl->bounded_produce ();
        });
        pimpl->m_playing = true;
      }
  }

  bool AudioPlayer::stream_portaudio ()
  {
    return false;
    /*
          require 'ruby-portaudio'
          PortAudio.init
          unless playing?
            @portaudio = pa::Stream.open( :sample_rate => 48000, :frames => 8192, :input => { :device => PortAudio::Device.default_input, :channels => 1, :sample_format => :int16, :suggested_latency => 0.05 })
            @audiobuffer = pa::SampleBuffer.new( :format => :float32, :channels => 1, :frames => @framesize)
            @portaudio.start
            spawn_threads :portaudio
            @playing = true
          end
          true
        rescue
          # no portaudio installed - no streaming possible
          false
        end
    */
  }

  void AudioPlayer::stop ()
  {
    if (pimpl->m_playing)
      {
        // kill_threads ();
        pimpl->m_encoder->reset ();
        if (!pimpl->m_file->closed ())
          {
            pimpl->m_file->close ();
          }
        /*
        if (!pimpl->m_portaudio.stopped ())
          {
                  pimpl->m_portaudio.stop ();
                }
        */
        pimpl->m_playing = false;
        pimpl->m_bounded_produce_thread.join ();
      }
  }

  void AudioPlayer::bitrate (size_t val)
  {
    if (!(pimpl->m_codec == Codec::alpha || pimpl->m_codec == Codec::beta))
      {
        try
          {
            pimpl->m_encoder->bitrate (val);
            pimpl->m_bitrate = val;
          }
        catch (...)
          {
          }
      }
  }

  void AudioPlayer::framelength (std::chrono::milliseconds ms)
  {
    using namespace std::chrono_literals;
    double framelength;
    if (ms >= 1ms && ms <= 4ms)
      {
        framelength = 2.5;
      }
    else if (ms > 4ms && ms <= 14ms)
      {
        framelength = 10.f;
      }
    else if (ms > 14ms && ms <= 30ms)
      {
        framelength = 20.f;
      }
    else if (ms > 30ms && ms <= 45ms)
      {
        framelength = 40.f;
      }
    else
      {
        framelength = 60.f;
      }
    pimpl->m_framesize = Impl::COMPRESSED_SIZE * framelength;
    try
      {
        pimpl->m_encoder->frame_size (pimpl->m_framesize);
        /*
        if (!m_portaudio.stopped ())
          {
            m_audiobuffer = std::make_unique <PortAudio::SampleBuffer> (pa::Format::float32, 1, m_framesize);
          }
        */
      }
    catch (...)
      {
      }
  }

  void AudioPlayer::volume (int vol)
  {
    pimpl->m_volume = vol;
  }

  int AudioPlayer::volume () const
  {
    return pimpl->m_volume;
  }

  int AudioPlayer::bitrate () const
  {
    return pimpl->m_bitrate;
  }

  std::chrono::milliseconds AudioPlayer::framelength ()
  {
    return std::chrono::milliseconds (pimpl->m_framesize / Impl::COMPRESSED_SIZE);
  }
}
