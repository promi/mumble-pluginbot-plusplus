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

namespace Mumble
{
  namespace wav = WaveFile;
  // namespace pa = PortAudio;

  AudioPlayer::AudioPlayer (Codec codec, Connection &connection,
                            size_t sample_rate, size_t bitrate)
    : m_conn (connection),
  /* m_wav_format (wav::Format (wav::Channels::mono, wav::SampleFormat::pcm_16,
     static_cast<size_t> (sample_rate))),*/
      m_codec (codec), m_bitrate (bitrate), m_sample_rate (sample_rate),
      m_framesize (COMPRESSED_SIZE * 10)
  {
    create_encoder (m_sample_rate, m_bitrate);
  }

  AudioPlayer::~AudioPlayer ()
  {
    m_bounded_produce_thread.join ();
    // kill_threads ()
  }

  void AudioPlayer::codec (Codec val)
  {
    m_codec = val;
    create_encoder (m_sample_rate, m_bitrate);
  }

  void AudioPlayer::play_file (const std::string &file)
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

  void AudioPlayer::stream_named_pipe (const std::string &file)
  {
    if (!m_playing)
      {
        m_file = std::make_unique<RawFileReader> (file);
        m_bounded_produce_thread = std::thread ([this] ()
        {
          bounded_produce ();
        });
        m_playing = true;
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
    if (m_playing)
      {
        // kill_threads ();
        m_encoder->reset ();
        if (!m_file->closed ())
          {
            m_file->close ();
          }
        /*
        if (!m_portaudio.stopped ())
          {
                  m_portaudio.stop ();
                }
        */
        m_playing = false;
        m_bounded_produce_thread.join ();
      }
  }

  void AudioPlayer::bitrate (size_t val)
  {
    if (!(m_codec == Codec::alpha || m_codec == Codec::beta))
      {
        try
          {
            m_encoder->bitrate (val);
            m_bitrate = val;
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
    m_framesize = COMPRESSED_SIZE * framelength;
    try
      {
        m_encoder->frame_size (m_framesize);
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

  void AudioPlayer::create_encoder (size_t sample_rate, size_t bitrate)
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
        // TODO: check over valid sample_rates (see opus-constants.h)
        assert (sample_rate == 48'000 /*'*/);
        m_encoder = std::make_unique<Opus::Encoder>
                    (static_cast<Opus::SampleRate> (sample_rate), m_framesize, Opus::Channels::mono,
                     7200);
        m_encoder->bitrate (bitrate);
        // constrainted vbr doesn't work with Opus::Signal::voice
        m_encoder->signal (Opus::Signal::music);
        m_encoder->vbr (true);
        m_encoder->vbr_constraint (true);
        m_encoder->packet_loss_percentage (10);
        m_encoder->dtx (true);
      }
  }

  std::vector<int16_t> AudioPlayer::change_volume (const std::vector<int16_t>
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

  void AudioPlayer::bounded_produce ()
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

  void AudioPlayer::produce ()
  {
    // TODO: Check if frame_size is in bytes or in samples
    encode_samples (m_file->read (m_encoder->frame_size ()));
    consume ();
  }

  void AudioPlayer::portaudio ()
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

  void AudioPlayer::encode_samples (const std::vector<int16_t> &samples)
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

  void AudioPlayer::consume (void)
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
    m_conn.send_udp_packet (packet.data (m_pds));
  }
}
