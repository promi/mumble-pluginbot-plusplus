/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2015 loscoala
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
#include "opus/encoder.hh"

namespace Opus
{
  Encoder::Encoder (SampleRate sample_rate, size_t frame_size, Channels channels,
                    size_t size)
    : m_frame_size (frame_size), m_size (size)
  {
    m_encoder = opus_encoder_create (static_cast<opus_int32> (sample_rate),
                                     static_cast<int> (channels),
                                     static_cast<int> (Application::audio), nullptr);
  }

  Encoder::~Encoder ()
  {
    opus_encoder_destroy (m_encoder);
  }

  void Encoder::reset ()
  {
    opus_encoder_ctl (m_encoder, OPUS_RESET_STATE);
  }

  void Encoder::vbr (bool enable)
  {
    opus_encoder_ctl (m_encoder, OPUS_SET_VBR(enable ? 1 : 0));
  }

  void Encoder::vbr_constraint (bool enable)
  {
    opus_encoder_ctl (m_encoder, OPUS_SET_VBR_CONSTRAINT(enable ? 1 : 0));
  }

  void Encoder::packet_loss_percentage (size_t percentage)
  {
    assert (percentage <= 100);
    opus_encoder_ctl (m_encoder, OPUS_SET_PACKET_LOSS_PERC(percentage));
  }

  void Encoder::bitrate (size_t bitrate)
  {
    opus_encoder_ctl (m_encoder, OPUS_SET_BITRATE(bitrate));
  }

  void Encoder::bitrate_max ()
  {
    opus_encoder_ctl (m_encoder, OPUS_SET_BITRATE(OPUS_BITRATE_MAX));
  }

  void Encoder::bitrate_auto ()
  {
    opus_encoder_ctl (m_encoder, OPUS_SET_BITRATE(OPUS_AUTO));
  }

  void Encoder::signal (Signal signal)
  {
    opus_encoder_ctl (m_encoder, OPUS_SET_SIGNAL(static_cast<opus_int32> (signal)));
  }

  std::vector<uint8_t> Encoder::encode (const std::vector<int16_t> &samples)
  {
    std::vector<uint8_t> out (m_size, 0);
    auto len = opus_encode (m_encoder, samples.data (), m_frame_size, out.data (),
                            m_size);
    if (len < 0 || static_cast<size_t> (len) != m_size)
      {
        throw std::string ("opus_encode returned wrong length");
      }
    return out;
  }

  /*
    def opus_get_complexity
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_COMPLEXITY_REQUEST
    end

    def opus_get_bitrate
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_BITRATE_REQUEST
    end

    def opus_set_complexity(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_COMPLEXITY_REQUEST, :int32, value
    end

    def opus_get_vbr
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_VBR_REQUEST
    end

    def opus_set_vbr_constraint(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_VBR_CONSTRAINT_REQUEST, :int32, value
    end

    def opus_get_vbr_constraint
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_VBR_CONSTRAINT_REQUEST
    end

    #* </dl>
    def opus_set_force_cannels(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_FORCE_CHANNELS_REQUEST, :int32, value
    end

    #define OPUS_GET_FORCE_CHANNELS(x) OPUS_GET_FORCE_CHANNELS_REQUEST, __opus_check_int_ptr(x)
    def opus_get_force_channels
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_FORCE_CHANNELS_REQUEST
    end

    #define OPUS_SET_MAX_BANDWIDTH(x) OPUS_SET_MAX_BANDWIDTH_REQUEST, __opus_check_int(x)
    def opus_set_max_bandwidth(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_MAX_BANDWIDTH_REQUEST, :int32, value
    end

    #define OPUS_GET_MAX_BANDWIDTH(x) OPUS_GET_MAX_BANDWIDTH_REQUEST, __opus_check_int_ptr(x)
    def opus_get_max_bandwidth
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_MAX_BANDWIDTH_REQUEST
    end

    #define OPUS_SET_BANDWIDTH(x) OPUS_SET_BANDWIDTH_REQUEST, __opus_check_int(x)
    def opus_set_bandwidth(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_BANDWIDTH_REQUEST, :int32, value
    end

    #define OPUS_SET_SIGNAL(x) OPUS_SET_SIGNAL_REQUEST, __opus_check_int(x)
    def opus_set_signal(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_SIGNAL_REQUEST, :int32, value
    end

    #define OPUS_GET_SIGNAL(x) OPUS_GET_SIGNAL_REQUEST, __opus_check_int_ptr(x)
    def opus_get_signal
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_SIGNAL_REQUEST
    end

    #define OPUS_SET_APPLICATION(x) OPUS_SET_APPLICATION_REQUEST, __opus_check_int(x)
    def opus_set_application(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_APPLICATION_REQUEST, :int32, value
    end

    #define OPUS_GET_APPLICATION(x) OPUS_GET_APPLICATION_REQUEST, __opus_check_int_ptr(x)
    def opus_get_application
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_APPLICATION_REQUEST, :int32, value
    end

    #
    #define OPUS_GET_SAMPLE_RATE(x) OPUS_GET_SAMPLE_RATE_REQUEST, __opus_check_int_ptr(x)
    def opus_encoder_get_sample_rate
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_SAMPLE_RATE_REQUEST
    end

    #
    #define OPUS_GET_LOOKAHEAD(x) OPUS_GET_LOOKAHEAD_REQUEST, __opus_check_int_ptr(x)
    def opus_get_lookahead
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_LOOKAHEAD_REQUEST
    end

    #define OPUS_SET_INBAND_FEC(x) OPUS_SET_INBAND_FEC_REQUEST, __opus_check_int(x)
    def opus_set_inband_fec(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_INBAND_FEC_REQUEST, :int32, value
    end

    #define OPUS_GET_INBAND_FEC(x) OPUS_GET_INBAND_FEC_REQUEST, __opus_check_int_ptr(x)
    def opus_get_inband_fec
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_INBAND_FEC_REQUEST
    end

    #define OPUS_SET_PACKET_LOSS_PERC(x) OPUS_SET_PACKET_LOSS_PERC_REQUEST, __opus_check_int(x)
    def opus_set_packet_loss_perc(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_PACKET_LOSS_PERC_REQUEST, :int32, value
    end

    #define OPUS_GET_PACKET_LOSS_PERC(x) OPUS_GET_PACKET_LOSS_PERC_REQUEST, __opus_check_int_ptr(x)
    def opus_get_packet_loss_perc
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_PACKET_LOSS_PERC_REQUEST
    end
  */

  void Encoder::dtx (bool enable)
  {
    opus_encoder_ctl (m_encoder, OPUS_SET_DTX(enable ? 1 : 0));
  }

  /*
    #define OPUS_GET_DTX(x) OPUS_GET_DTX_REQUEST, __opus_check_int_ptr(x)
    def opus_get_dtx
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_DTX_REQUEST
    end

    #define OPUS_SET_LSB_DEPTH(x) OPUS_SET_LSB_DEPTH_REQUEST, __opus_check_int(x)
    def opus_set_lsb_depth(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_LSB_DEPTH_REQUEST, :int32, value
    end

    #define OPUS_GET_LSB_DEPTH(x) OPUS_GET_LSB_DEPTH_REQUEST, __opus_check_int_ptr(x)
    def opus_get_lsb_depth
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_LSB_DEPTH_REQUEST
    end

    #define OPUS_GET_LAST_PACKET_DURATION(x) OPUS_GET_LAST_PACKET_DURATION_REQUEST, __opus_check_int_ptr(x)
    def opus_get_last_packet_duration
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_LAST_PACKET_DURATION_REQUEST
    end

    #define OPUS_SET_EXPERT_FRAME_DURATION(x) OPUS_SET_EXPERT_FRAME_DURATION_REQUEST, __opus_check_int(x)
    def opus_set_expert_frame_duration(value)
       Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_EXPERT_FRAME_DURATION_REQUEST, :int32, value
    end

    #define OPUS_GET_EXPERT_FRAME_DURATION(x) OPUS_GET_EXPERT_FRAME_DURATION_REQUEST, __opus_check_int_ptr(x)
    def opus_get_expert_frame_duration
       Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_EXPERT_FRAME_DURATION_REQUEST
    end

    #define OPUS_SET_PREDICTION_DISABLED(x) OPUS_SET_PREDICTION_DISABLED_REQUEST, __opus_check_int(x)
    def opus_set_prediction_disabled(value)
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_SET_PREDICTION_DISABLED_REQUEST, :int32, value
    end

    #define OPUS_GET_PREDICTION_DISABLED(x) OPUS_GET_PREDICTION_DISABLED_REQUEST, __opus_check_int_ptr(x)
    def opus_get_prediction_disabled
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_PREDICTION_DISABLED_REQUEST
    end

    #define OPUS_GET_BANDWIDTH(x) OPUS_GET_BANDWIDTH_REQUEST, __opus_check_int_ptr(x)
    def opus_encoder_get_bandwidth
      Opus.opus_encoder_ctl @encoder, Opus::Constants::OPUS_GET_BANDWIDTH_REQUEST
    end
  */

}
