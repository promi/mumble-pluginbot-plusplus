/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2012 Matthew Perry (mattvperry)
    Copyright (c) 2013 Jeromy Maligie (Kingles)
    Copyright (c) 2013 Matthew Perry (mattvperry)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 niko20010
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
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <memory>
#include <google/protobuf/message.h>

#include "Mumble.pb.h"

namespace Mumble
{
  class Messages
  {
  public:
    static std::unordered_map<std::type_index, uint16_t> sym_to_type;
    static std::unique_ptr<::google::protobuf::Message> msg_from_type (
      uint16_t type);
  };

}
