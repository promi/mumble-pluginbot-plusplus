/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Jack Chen (chendo)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 loscoala
    Copyright (c) 2015 Georg G. (nilsding)
    Copyright (c) 2015 Karsten Nerdinger (Piratonym)
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

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include "Mumble.pb.h"

namespace Mumble
{
  class User
  {
  private:
    uint32_t m_session;
    std::string m_name;
    uint32_t m_user_id = 0;
    bool m_has_user_id;
    uint32_t m_channel_id = 0;
    bool m_mute = false;
    bool m_deaf = false;
    bool m_suppress = false;
    bool m_self_mute = false;
    bool m_self_deaf = false;
    std::vector<uint8_t> m_texture;
    //std::vector<uint8_t> m_plugin_context;
    //std::string m_plugin_identity;
    std::string m_comment;
    std::string m_hash;
    std::vector<uint8_t> m_comment_hash;
    std::vector<uint8_t> m_texture_hash;
    bool m_priority_speaker = false;
    bool m_recording = false;
  public:
    inline User (const MumbleProto::UserState &msg)
    {
      update (msg);
    }
    inline auto session () const
    {
      return m_session;
    }
    inline auto name () const
    {
      return m_name;
    }
    inline auto user_id () const
    {
      return m_user_id;
    }
    inline auto has_user_id () const
    {
      return m_has_user_id;
    }
    inline auto channel_id () const
    {
      return m_channel_id;
    }
    inline auto mute () const
    {
      return m_mute;
    }
    inline auto deaf () const
    {
      return m_deaf;
    }
    inline auto suppress () const
    {
      return m_suppress;
    }
    inline auto self_mute () const
    {
      return m_self_mute;
    }
    inline auto self_deaf () const
    {
      return m_self_deaf;
    }
    inline auto texture () const
    {
      return m_texture;
    }
    inline auto comment () const
    {
      return m_comment;
    }
    inline auto hash () const
    {
      return m_hash;
    }
    inline auto comment_hash () const
    {
      return m_comment_hash;
    }
    inline auto texture_hash () const
    {
      return m_texture_hash;
    }
    inline auto priority_speaker () const
    {
      return m_priority_speaker;
    }
    inline auto recording () const
    {
      return m_recording;
    }
    void update (const MumbleProto::UserState &msg);
  };
}

