/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
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
#include "mumble/audio-recorder.hh"

namespace Mumble
{
  AudioRecorder::AudioRecorder (ClientIntf &client, size_t sample_rate)
    : m_client (client), m_sample_rate (
      sample_rate) /* m_wav_format (WaveFile::Format (mono, pcm_16, sample_rate)) */
  {
  }

  void AudioRecorder::stream_portaudio ()
  {
    /*
        PortAudio.init
        unless !recording?
          @portaudio = PortAudio::Stream.open( :sample_rate => 48000, :frames => 8192, :output => { :device => PortAudio::Device.default_output, :channels => 1, :sample_format => :int16, :suggested_latency => 0.25 })
          @audiobuffer = PortAudio::SampleBuffer.new(:format   => :int16, :channels => 1, :frames => 8192)
          @portaudio.start
          @callback = @client.on_udp_tunnel { |msg| process_udp_tunnel msg }
          spawn_thread :portaudio
          @recording = true
        end
        true
      rescue
        # no portaudio gem installed - no streaming possible.
        false
        end
    */
  }

  void AudioRecorder::start (const std::string &file)
  {
    if (m_recording)
      {
        return;
      }
    /*
    m_file = std::make_unique<WaveFile::Writer> (m_filename, m_wav_format);
    m_callback = m_client.on_udp_tunnel { |msg| process_udp_tunnel msg }
        spawn_thread :write_audio
        @recording = true
      end
    */
  }

  void AudioRecorder::stop ()
  {
    if (!m_recording)
      {
        return;
      }
    /*
        @client.remove_callback :udp_tunnel, @callback
        kill_threads
        @opus_decoders.values.each &:destroy
        @opus_decoders.clear
        @queues.clear
        @file.close
        @recording = false
    */
  }

  /*
    private
    def process_udp_tunnel(message)
      @pds_lock.synchronize do
        @pds.rewind
        @pds.append_block message.packet#[1..-1]        # we need packet type info

        @pds.rewind
        packet_type = @pds.get_next
        source = @pds.get_int
        seq = @pds.get_int
        case ( packet_type >> 5 )
          when Codec::alpha
            len = @pds.get_next
            alpha = @pds.get_block ( len & 0x7f )
            @queues[source] << @celt_decoders[source].decode(alpha.join)
            while ( len  0x80 ) != 0
              len = @pds.get_next
              alpha = @pds.get_block ( len & 0x7f )
              @queues[source] << @celt_decoders[source].decode(alpha.join)
            end
          when Codec::beta
            len = @pds.get_next
            beta = @pds.get_block ( len & 0x7f )
            @queues[source] << @celt_decoders[source].decode(beta.join)
            while ( len  0x80 ) != 0
              len = @pds.get_next
              beta = @pds.get_block ( len & 0x7f )
              @queues[source] << @celt_decoders[source].decode(beta.join)
            end
          when Codec::opus
            len = @pds.get_int
            opus = @pds.get_block ( len & 0x1FFF )
            @queues[source] << @opus_decoders[source].decode(opus.join)
            @opus_decoders[source].reset if ( len & 0x2000 ) == 0x2000
        end
      end
    end


    def initialize(client, sample_rate)
        h[k] = Opus::Decoder.new sample_rate, sample_rate / 100, 1
        h[k] = Celt::Decoder.new sample_rate, sample_rate / 100, 1
    end

    # TODO: Better audio stream merge with normalization
    def write_audio
      pcms = @queues.values
        .reject { |q| q.empty? }                      # Remove empty queues
        .map { |q| q.pop.unpack 's*' }                # Grab the top element of each queue and expand

      head, *tail = pcms
      if head
        samples = head.zip(*tail)
          .map { |pcms| pcms.reduce(:+) / pcms.size }   # Average together all the columns of the matrix (merge audio streams)
          .flatten                                      # Flatten the resulting 1d matrix
        @file.write WaveFile::Buffer.new(samples, @wav_format)
      end
    end

    def portaudio
      pcms = @queues.values
        .reject { |q| q.empty? }                      # Remove empty queues
        .map { |q| q.pop.unpack 's*' }                # Grab the top element of each queue and expand
      head, *tail = pcms
      if head
        samples = head.zip(*tail)
          .map { |pcms| pcms.reduce(:+) / pcms.size }   # Average together all the columns of the matrix (merge audio streams)
          .flatten                                      # Flatten the resulting 1d matrix
        @audiobuffer = PortAudio::SampleBuffer.new( :format => :int16, :channels => 1, :frames => send.size / 2 +1)
        @audiobuffer.add(sample.pack 's*')
        begin
          @portaudio << @audiobuffer
        rescue
        end
      end
    end
  end
  end
  */
}
