/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Jack Chen (chendo)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
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

#include <string>
#include <list>

#include "Mumble.pb.h"

namespace Mumble
{
  class Channel
  {
  private:
    uint32_t m_channel_id;
    uint32_t m_parent = 0;
    std::string m_name;
    std::list<uint32_t> m_links;
    std::string m_description;
    bool m_temporary = false;
    int32_t m_position = 0;
    std::string m_description_hash;
    uint32_t m_max_users;
    bool m_max_users_set = false;
  public:
    inline Channel (const MumbleProto::ChannelState &msg)
    {
      update (msg);
    }
    inline auto channel_id () const
    {
      return m_channel_id;
    }
    inline auto parent () const
    {
      return m_parent;
    }
    inline auto name () const
    {
      return m_name;
    }
    inline auto links () const
    {
      return m_links;
    }
    inline auto description () const
    {
      return m_description;
    }
    inline auto temporary () const
    {
      return m_temporary;
    }
    inline auto description_hash () const
    {
      return m_description_hash;
    }
    inline auto max_users () const
    {
      return m_max_users;
    }
    inline auto max_users_set () const
    {
      return m_max_users_set;
    }
    void update (const MumbleProto::ChannelState &msg);
  };
}

