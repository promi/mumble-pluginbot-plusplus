/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2012 Matthew Perry (mattvperry)
    Copyright (c) 2014 niko20010
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
#include "mumble/packet-data-stream.hh"

namespace Mumble
{
  PacketDataStream::PacketDataStream (size_t capacity) : m_data (capacity, 0),
    m_capacity (capacity)
  {
  }

  PacketDataStream::PacketDataStream (const std::vector<uint8_t> &data) : m_data (
      data), m_capacity (data.size ())
  {
  }

  void PacketDataStream::append (uint8_t val)
  {
    if (m_pos < m_capacity)
      {
        m_data[m_pos++] = val;
      }
    else
      {
        m_ok = false;
      }
  }

  void PacketDataStream::append_block (const std::vector<uint8_t> &data)
  {
    auto len = data.size ();
    if (len < left ())
      {
        for (auto val : data)
          {
            m_data[m_pos++] = val;
          }
      }
    else
      {
        m_ok = false;
      }
  }

  std::vector<uint8_t> PacketDataStream::get_block (size_t len)
  {
    std::vector<uint8_t> block;
    if (len < left ())
      {
        for (size_t i = 0; i < len; i++)
          {
            block.push_back (m_data[m_pos++]);
          }
      }
    else
      {
        m_ok = false;
      }
    return block;
  }

  uint8_t PacketDataStream::get_next ()
  {
    if (m_pos < m_capacity)
      {
        return m_data[m_pos++];
      }
    else
      {
        m_ok = false;
        return 0;
      }
  }

  void PacketDataStream::put_int (int64_t val)
  {
    if (((val & 0x8000000000000000) != 0) && (~val < 0x100000000))
      {
        val = ~val;
        // puts int
        if (val <= 0x3)
          {
            // Shortcase for -1 to -4
            append(0xFC | val);
          }
        else
          {
            append(0xF8);
          }
      }

    if (val < 0x80)
      {
        // Need top bit clear
        append (val);
      }
    else if (val < 0x4000)
      {
        // Need top two bits clear
        append ((val >> 8) | 0x80);
        append (val & 0xFF);
      }
    else if (val < 0x200000)
      {
        // Need top three bits clear
        append ((val >> 16) | 0xC0);
        append ((val >> 8) & 0xFF);
        append (val & 0xFF);
      }
    else if (val < 0x10000000)
      {
        // Need top four bits clear
        append ((val >> 24) | 0xE0);
        append ((val >> 16) & 0xFF);
        append ((val >> 8) & 0xFF);
        append (val & 0xFF);
      }
    else if (val < 0x100000000)
      {
        // It's a full 32-bit integer.
        append (0xF0);
        append ((val >> 24) & 0xFF);
        append ((val >> 16) & 0xFF);
        append ((val >> 8) & 0xFF);
        append (val & 0xFF);
      }
    else
      {
        // It's a 64-bit value.
        append (0xF4);
        append ((val >> 56) & 0xFF);
        append ((val >> 48) & 0xFF);
        append ((val >> 40) & 0xFF);
        append ((val >> 32) & 0xFF);
        append ((val >> 24) & 0xFF);
        append ((val >> 16) & 0xFF);
        append ((val >> 8) & 0xFF);
        append (val & 0xFF);
      }
  }

  int64_t PacketDataStream::get_int ()
  {
    uint8_t v = get_next ();
    int64_t val = 0;

    if ((v & 0x80) == 0x00)
      {
        val = v & 0x7F;
      }
    else if ( (v & 0xC0) == 0x80)
      {
        val = (v & 0x3F) << 8 | get_next ();
      }
    else if ( (v & 0xF0) == 0xF0)
      {
        uint8_t x = v & 0xFC;
        if (x == 0xF0)
          {
            val = get_next () << 24 | get_next () << 16 | get_next () << 8 | get_next ();
          }
        else if ( x == 0xF4)
          {
            val = (int64_t) get_next () << 56 | (int64_t) get_next () << 48 |
                  (int64_t) get_next () << 40 | (int64_t) get_next () <<
                  32 |
                  get_next () << 24 | get_next () << 16 | get_next () << 8  | get_next ();
          }
        else if ( x == 0xF8)
          {
            val = get_int ();
            val = ~val;
          }
        else if ( x == 0xFC)
          {
            val = v & 0x03;
            val = ~val;
          }
        else
          {
            m_ok = false;
            val = 0;
          }
      }
    else if ( (v & 0xF0) == 0xE0)
      {
        val = (v & 0x0F) << 24 | get_next () << 16 | get_next () << 8 | get_next ();
      }
    else if ( (v & 0xE0) == 0xC0)
      {
        val = (v & 0x1F) << 16 | get_next () << 8 | get_next ();
      }
    return val;
  }

}
