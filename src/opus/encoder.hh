/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
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
#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <opus/opus.h>

#include "opus/constants.hh"

namespace Opus
{
  class Encoder
  {
    //  attr_reader :sample_rate, :frame_size, :channels, :vbr_rate, :vbr_constraint, :bitrate, :signal
  private:
    size_t m_frame_size;
    size_t m_size;
    OpusEncoder *m_encoder;
  public:
    Encoder (SampleRate sample_rate, size_t frame_size, Channels channels,
             size_t size);
    ~Encoder ();
    void reset ();
    void dtx (bool enable);
    void vbr (bool enable);
    void vbr_constraint (bool enable);
    void packet_loss_percentage (size_t percentage);
    void bitrate (size_t bitrate);
    void bitrate_max ();
    void bitrate_auto ();
    void signal (Signal signal);
    inline size_t frame_size (void) const
    {
      return m_frame_size;
    }
    inline void frame_size (size_t frame_size)
    {
      m_frame_size = frame_size;
    }
    std::vector<uint8_t> encode (const std::vector<int16_t> &samples);
  };
}

/*
    def vbr_rate=(value)
    def vbr_contstraint=(value)
    def packet_loss_perc=(value)
    def bitrate=(value)
    def signal=(value)
    def set_frame_size frame_size
    def encode(data)
    def encode_ptr(memorypointer)
    def opus_get_complexity
    def opus_get_bitrate
    def opus_set_complexity(value)
    def opus_set_bitrate(value)
    def opus_set_vbr(value)
    def opus_get_vbr
    def opus_set_vbr_constraint(value)
    def opus_get_vbr_constraint
    def opus_set_force_cannels(value)
    def opus_get_force_channels
    def opus_set_max_bandwidth(value)
    def opus_get_max_bandwidth
    def opus_set_bandwidth(value)
    def opus_set_signal(value)
    def opus_get_signal
    def opus_set_application(value)
    def opus_get_application
    def opus_encoder_get_sample_rate
    def opus_get_lookahead
    def opus_set_inband_fec(value)
    def opus_get_inband_fec
    def opus_set_packet_loss_perc(value)
    def opus_get_packet_loss_perc
    def opus_set_dtx(value)
    def opus_get_dtx
    def opus_set_lsb_depth(value)
    def opus_get_lsb_depth
    def opus_get_last_packet_duration
    def opus_set_expert_frame_duration(value)
    def opus_get_expert_frame_duration
    def opus_set_prediction_disabled(value)
    def opus_get_prediction_disabled
    def opus_encoder_get_bandwidth
*/
