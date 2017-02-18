/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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
#include "util/endian.hh"
#include "io/raw-s16le-file-sample-reader.hh"

RawFileReader::RawFileReader (const FileSystem::path &file)
  : m_stream (file.string (), std::ios::binary)
{
}

std::vector<int16_t> RawFileReader::internal_read (size_t count)
{
  auto size = count * 2;
  std::vector<uint8_t> buffer (size, 0);
  m_stream.read (reinterpret_cast<char*> (buffer.data ()), size);
  if (m_stream.gcount () == 0)
    {
      buffer.clear ();
    }
  else if (static_cast<size_t> (m_stream.gcount ()) != size)
    {
      throw std::runtime_error ("failed to read enough samples from stream");
    }
  return EndianUtils::from_s16le (buffer);
}

void RawFileReader::internal_each_buffer (size_t count,
    std::function<bool(const std::vector<int16_t>&)> f)
{
  decltype(internal_read (count)) samples;
  while ((samples = internal_read (count)).size () != 0)
    {
      f (samples);
    }
}

void RawFileReader::internal_close ()
{
  m_stream.close ();
  SampleReader::close ();
}
