/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2012 Matthew Perry (mattvperry)
    Copyright (c) 2013 Jeromy Maligie (Kingles)
    Copyright (c) 2013 Matthew Perry (mattvperry)
    Copyright (c) 2014 Aaron Herting (qwertos)
    Copyright (c) 2014 Benjamin Neff (SuperTux88)
    Copyright (c) 2014 Jack Chen (chendo)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 Meister der Bots
    Copyright (c) 2014 dafoxia
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 loscoala
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
#include <string>
#include <sstream>

namespace Mumble
{
  // The integral values assigned here are used in the UDP (and UDPTunnel)
  // packet stream. Do not change!
  enum class Codec : uint8_t
  {
    alpha = 0,
    // Not a codec, but is used for UDP PING packets
    ping = 1,
    speex = 2,
    beta = 3,
    opus = 4
  };

  inline std::string to_string (Codec codec)
  {
    switch (codec)
      {
      case Codec::alpha:
        return "CELT-ALPHA (0.7.0)";
      case Codec::ping:
        return "PING";
      case Codec::speex:
        return "SPEEX";
      case Codec::beta:
        return "CELT-BETA (0.11.0)";
      case Codec::opus:
        return "OPUS";
      default:
        std::stringstream ss;
        ss << "[ERROR] Unknown codec: " << static_cast<uint8_t> (codec);
        return ss.str ();
      }
  }
}
