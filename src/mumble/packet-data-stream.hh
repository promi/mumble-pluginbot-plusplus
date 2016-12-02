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
#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace Mumble
{
  class PacketDataStream
  {
  private:
    std::vector<uint8_t> m_data;
    size_t m_capacity;
    size_t m_pos = 0;
    bool m_ok = true;
  public:
    PacketDataStream (size_t capacity = 8192);
    PacketDataStream (const std::vector<uint8_t> &data);
    inline bool valid ()
    {
      return m_ok;
    }
    inline size_t size ()
    {
      return m_pos;
    }
    inline size_t left ()
    {
      return m_capacity - m_pos;
    }
    void append (uint8_t val);
    uint8_t get_next ();
    void append_block (const std::vector<uint8_t> &data);
    std::vector<uint8_t> get_block (size_t len);
    inline void rewind ()
    {
      m_pos = 0;
    }
    void skip (size_t len = 1)
    {
      if (len < left ())
        {
          m_pos += len;
        }
      else
        {
          m_ok = false;
        }
    }
    void put_int (int64_t val);
    int64_t get_int ();
  };
}
