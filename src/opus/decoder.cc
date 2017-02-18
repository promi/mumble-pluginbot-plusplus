/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Aaron Herting (qwertos)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2015 loscoala
    Copyright (c) 2015 dafoxia
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
#include "opus/decoder.hh"

namespace Opus
{
  Decoder::Decoder(size_t sample_rate, size_t frame_size, size_t channels)
    : m_frame_size (frame_size), m_channels (channels)
  {
    m_decoder = opus_decoder_create (sample_rate, channels, nullptr);
  }

  Decoder::~Decoder ()
  {
    opus_decoder_destroy (m_decoder);
  }

  void Decoder::reset ()
  {
    opus_decoder_ctl (m_decoder, OPUS_RESET_STATE);
  }

  std::vector<int16_t> Decoder::decode (const std::vector<uint8_t> &data)
  {
    static_assert(sizeof(opus_int16) == sizeof(int16_t), "opus_int16 != int16_t");
    std::vector<int16_t> samples (m_frame_size * m_channels, 0);
    auto sample_count = opus_decode (m_decoder, data.data (), data.size (),
                                     samples.data (),
                                     samples.size () * sizeof(int16_t), 0);
    if (sample_count < 0)
      {
        m_lasterror = sample_count;
        sample_count = 0;
      }
    samples.resize (sample_count);
    return samples;
  }

  /*
  void Decoder::decode_missed ()
  {
    // This is incomplete, needs to pass
    // the exact missing duration size and do something with the output
    // See documentation for opus_decode ()
    opus_decode (m_decoder, nullptr, 0, nullptr, 0, 0);
  }
  */

  std::string Decoder::lasterror_text ()
  {
    switch (m_lasterror)
      {
      case 0:
        return "OK";
      case -1:
        return "BAD ARG";
      case -2:
        return "BUFFER TOO SMALL";
      case -3:
        return "INTERNAL ERROR";
      case -4:
        return "INVALID PACKET";
      case -5:
        return "UNIMPLEMENTED";
      case -6:
        return "INVALID STATE";
      case -7:
        return "ALLOC FAIL";
      default:
        return "UNKNOWN ERROR";
      }
  }
}
