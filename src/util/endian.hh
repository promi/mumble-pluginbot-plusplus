/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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
#include <vector>
#include <cstddef>
#include <cstdint>

const int be_test_var = 1;
#define is_bigendian() ( (*(char*)&be_test_var) == 0 )

class EndianUtils
{
public:
  static inline std::vector<int16_t> from_s16le (const std::vector<uint8_t>
      &bytes)
  {
    auto ptr = reinterpret_cast<const uint8_t*> (bytes.data ());
    if (!is_bigendian ())
      {
        return std::vector<int16_t>(ptr, ptr + bytes.size ());
      }
    else
      {
        std::vector<int16_t> v;
        for (size_t i = 0; i < bytes.size () / 2; i +=2)
          {
            v.push_back (
                         ptr[i + 1] << 8 |
                         ptr[i + 0] << 0
                         );
          }
        return v;
      }
  }

  static inline uint16_t value_from_u16be (const std::vector<uint8_t> &bytes,
      size_t offset)
  {
    assert (bytes.size () >= offset + 2);
    if (is_bigendian ())
      {
        return reinterpret_cast<const uint16_t*> (bytes.data()) [offset];
      }
    else
      {
        return
          bytes[offset + 0] << 8 |
          bytes[offset + 1] << 0;
      }
  }

  static inline uint32_t value_from_u32be (const std::vector<uint8_t> &bytes,
      size_t offset)
  {
    assert (bytes.size () >= offset + 4);
    if (is_bigendian ())
      {
        return reinterpret_cast<const uint32_t*> (bytes.data()) [offset];
      }
    else
      {
        return
          bytes[offset + 0] << 24 |
          bytes[offset + 1] << 16 |
          bytes[offset + 2] << 8 |
          bytes[offset + 3] << 0;
      }
  }

  static inline void add_to_u16be (std::vector<uint8_t> &bytes, uint16_t value)
  {
    if (is_bigendian ())
      {
        bytes.push_back (value >> 0);
        bytes.push_back (value >> 8);
      }
    else
      {
        bytes.push_back (value >> 8);
        bytes.push_back (value >> 0);
      }
  }

  static inline void add_to_u32be (std::vector<uint8_t> &bytes, uint32_t value)
  {
    if (is_bigendian ())
      {
        bytes.push_back (value >> 0);
        bytes.push_back (value >> 8);
        bytes.push_back (value >> 16);
        bytes.push_back (value >> 24);
      }
    else
      {
        bytes.push_back (value >> 24);
        bytes.push_back (value >> 16);
        bytes.push_back (value >> 8);
        bytes.push_back (value >> 0);
      }
  }
};
