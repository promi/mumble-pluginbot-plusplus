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

#include <string>
#include <fstream>

#include "filesystem/filesystem.hh"
#include "io/sample-reader.hh"

class RawFileReader : public SampleReader
{
private:
  std::ifstream m_stream;
protected:
  std::vector<int16_t> internal_read (size_t count) override;
  void internal_each_buffer (size_t count,
                             std::function<bool(const std::vector<int16_t>&)> f) override;
  void internal_close () override;
public:
  RawFileReader (const FileSystem::path &file);
};
