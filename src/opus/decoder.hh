/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
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
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

#include <opus/opus.h>

namespace Opus
{
  class Decoder
  {
  private:
    size_t m_frame_size;
    size_t m_channels;
    OpusDecoder *m_decoder;
    int32_t m_lasterror;
  public:
    Decoder (size_t sample_rate, size_t frame_size, size_t channels);
    ~Decoder ();
    void reset ();
    std::vector<int16_t> decode (const std::vector<uint8_t> &data);
    void decode_missed ();
    inline auto lasterror () const
    {
      return m_lasterror;
    }
    std::string lasterror_text ();
  };
}
