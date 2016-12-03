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

#include <vector>
#include <functional>

class SampleWriter
{
private:
  //bool m_closed = false;
protected:
  //virtual std::vector<int16_t> internal_read (size_t count) = 0;
  //virtual void internal_each_buffer (size_t count, std::function<bool(const std::vector<int16_t>&)> f) = 0;
  // virtual void internal_close ()
  //{
  //  m_closed = true;
  //}
public:
  virtual ~SampleWriter ();
  /*
  inline bool closed () const
  {
    return m_closed;
  }
  inline std::vector<int16_t> read (size_t count)
  {
    return internal_read (count);
  }
  inline void each_buffer (size_t count,
                           std::function<bool(const std::vector<int16_t>&)> f)
  {
    internal_each_buffer (count, f);
  }
  inline void close ()
  {
    internal_close ();
  }
  */
};
