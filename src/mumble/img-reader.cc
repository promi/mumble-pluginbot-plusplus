/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 niko20010
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
#include <fstream>
#include <sstream>
#include <vector>

#include "util/base64.hh"
#include "mumble/img-reader.hh"

namespace fs = std::experimental::filesystem;

namespace Mumble
{
  const std::list<std::string> ImgReader::FORMATS = {"png", "jpg", "jpeg", "svg"};

  std::string ImgReader::msg_from_file (const fs::path &file)
  {
    auto p = extension_valid (file.extension ());
    if (!p.first)
      {
        throw std::runtime_error ("Image format must be on of the following: " +
                                  formats_to_string ());
      }
    std::ifstream is (file);
    if (is.fail ())
      {
        throw std::runtime_error (std::string ("File could not be opened for reading: ")
                                  + file.string ());
      }
    auto size = is.tellg ();
    if (size > 128 * 1024)
      {
        throw std::runtime_error ("Image must be smaller than 128 KiB");
      }
    std::vector<uint8_t> data (size, 0);
    is.read (reinterpret_cast<char*> (data.data ()), size);

    std::stringstream res;
    res << "<img src='data:image/" << p.second << ";base64," << base64_encode (
          data) << "'/>";
    return res.str ();
  }

  std::pair<bool, std::string> ImgReader::extension_valid (
    const std::string &extension)
  {
    for (auto& fmt : FORMATS)
      {
        if (extension.compare (extension.size () - fmt.size (), fmt.size (), fmt) == 0)
          {
            return std::make_pair (true, fmt);
          }
      }
    return std::make_pair (false, "");
  }

  std::string ImgReader::formats_to_string ()
  {
    std::stringstream ss;
    ss << "{";
    bool fst = true;
    for (auto& fmt : FORMATS)
      {
        if (fst)
          {
            ss << fmt;
            fst = false;
          }
        else
          {
            ss << ", " << fmt;
          }
      }
    ss << "}";
    return ss.str ();
  }
}
