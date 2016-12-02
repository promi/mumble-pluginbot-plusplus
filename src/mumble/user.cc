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
#include "mumble/user.hh"

namespace Mumble
{
  void User::update (const MumbleProto::UserState &msg)
  {
    if (msg.has_session ())
      {
        m_session = msg.session ();
      }
    if (msg.has_name ())
      {
        m_name = msg.name ();
      }
    if (msg.has_user_id ())
      {
        m_user_id = msg.user_id ();
        m_has_user_id = true;
      }
    if (msg.has_channel_id ())
      {
        m_channel_id = msg.channel_id ();
      }
    if (msg.has_mute ())
      {
        m_mute = msg.mute ();
      }
    if (msg.has_deaf ())
      {
        m_deaf = msg.deaf ();
      }
    if (msg.has_suppress ())
      {
        m_suppress = msg.suppress ();
      }
    if (msg.has_self_mute ())
      {
        m_self_mute = msg.self_mute ();
      }
    if (msg.has_self_deaf ())
      {
        m_self_deaf = msg.self_deaf ();
      }
    if (msg.has_texture ())
      {
        m_texture.clear ();
        const auto& tmp = msg.texture ();
        std::copy (std::begin (tmp), std::end (tmp), std::back_inserter (m_texture));
      }
    if (msg.has_comment ())
      {
        m_comment = msg.comment ();
      }
    if (msg.has_hash ())
      {
        m_hash = msg.hash ();
      }
    if (msg.has_comment_hash ())
      {
        m_comment_hash.clear ();
        const auto& tmp = msg.comment_hash ();
        std::copy (std::begin (tmp), std::end (tmp),
                   std::back_inserter (m_comment_hash));
      }
    if (msg.has_texture_hash ())
      {
        m_texture_hash.clear ();
        const auto& tmp = msg.texture_hash ();
        std::copy (std::begin (tmp), std::end (tmp),
                   std::back_inserter (m_texture_hash));
      }
    if (msg.has_priority_speaker ())
      {
        m_priority_speaker = msg.priority_speaker ();
      }
    if (msg.has_recording ())
      {
        m_recording = msg.recording ();
      }
  }
}
