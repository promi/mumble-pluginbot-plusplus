/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2016 Manuel Martinez
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
#include <string>

// Original source from stackoverflow: http://stackoverflow.com/a/34571089/426242

inline std::string base64_encode(const std::vector<uint8_t> &in)
{
  std::string out;

  int val = 0, valb = -6;
  for (uint8_t c : in)
    {
      val = (val << 8) + c;
      valb += 8;
      while (valb >= 0)
        {
          out.push_back ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val
                         >> valb) & 0x3F]);
          valb -= 6;
        }
    }
  if (valb > -6)
    {
      out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((
                      val << 8) >> (valb + 8)) & 0x3F]);
    }
  while (out.size() % 4)
    {
      out.push_back('=');
    }
  return out;
}

/*
inline std::string base64_decode(const std::string &in) {

  std::string out;

  std::vector<int> T(256,-1);
  for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

  int val=0, valb=-8;
  for (uchar c : in) {
      if (T[c] == -1) break;
      val = (val<<6) + T[c];
      valb += 6;
      if (valb>=0) {
          out.push_back(char((val>>valb)&0xFF));
          valb-=8;
      }
  }
  return out;
}
*/
