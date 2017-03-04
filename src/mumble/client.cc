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
#include "mumble/client-impl.hh"

#include <openssl/sha.h>
#include "mumble/img-reader.hh"

namespace Mumble
{
  Client::Client (const Aither::Log &log, const Configuration &config,
                  const Certificate &cert,
                  const std::string &client_identification)
    : pimpl (std::make_unique<Impl> (log, config, cert, client_identification))
  {
  }

  Client::~Client ()
  {
  }

  bool Client::connect ()
  {
    return pimpl->connect ();
  }

  void Client::disconnect ()
  {
    pimpl->disconnect ();
  }

  bool Client::connected () const
  {
    return pimpl->m_connected;
  }

  Connection& Client::connection () const
  {
    if (pimpl->m_connection == nullptr)
      {
        throw std::runtime_error ("No connection available");
      }
    return *pimpl->m_connection;
  }

  bool Client::synced () const
  {
    return pimpl->m_synced;
  }

  std::string Client::codec () const
  {
    if (!pimpl->m_codec_usable)
      {
        throw std::runtime_error ("no usable codec");
      }
    return to_string (pimpl->m_codec);
  }

  Codec Client::codec_int () const
  {
    return pimpl->m_codec;
  }

  void Client::deaf (bool b)
  {
    MumbleProto::UserState message;
    message.set_self_deaf (b);
    send (message);
  }

  void Client::mute (bool b)
  {
    MumbleProto::UserState message;
    message.set_self_mute (b);
    send (message);
  }

  User& Client::me ()
  {
    return pimpl->m_users.at (pimpl->m_session);
  }

  void Client::comment (const std::string &newcomment)
  {
    MumbleProto::UserState message;
    message.set_comment (newcomment);
    send (message);
  }

  void Client::join_channel (const std::string &name)
  {
    Channel *channel = find_channel (name);
    if (!channel)
      {
        throw std::runtime_error ("Channel not found: " + name);
      }
    uint32_t channel_id = channel->channel_id ();
    MumbleProto::UserState msg;
    msg.set_session (pimpl->m_session);
    msg.set_channel_id (channel_id);
    send (msg);
  }

  void Client::text_user (uint32_t session, const std::string &message)
  {
    MumbleProto::TextMessage msg;
    msg.add_session (session);
    msg.set_message (message);
    send (msg);
  }

  void Client::text_user_img (uint32_t session, const FileSystem::path &file)
  {
    text_user (session, ImgReader::msg_from_file (file));
  }

  void Client::text_channel (uint32_t channel_id, const std::string &message)
  {
    MumbleProto::TextMessage msg;
    msg.add_channel_id (channel_id);
    msg.set_message (message);
    send (msg);
  }

  void Client::text_channel_img (uint32_t channel_id,
                                 const FileSystem::path &file)
  {
    text_channel (channel_id, ImgReader::msg_from_file (file));
  }

  void Client::avatar (const std::string &img)
  {
    // first set Image, then update Hash
    // imagesize is max 600x60
    // has to be a PNG with AlphaChannel and 32 Bit deepth (ARGB format)
    // SHA1 assumes a 20 byte array, leave room for 0 termination char as well!
    uint8_t hash[21] = {0};
    SHA1 (reinterpret_cast<const uint8_t*> (img.data()), img.size (), hash);
    MumbleProto::UserState msg;
    msg.set_texture (img);
    msg.set_texture_hash (reinterpret_cast<const char*> (hash));
    send (msg);
  }

  void Client::avatar (uint32_t id)
  {
    // send a request for avatar image
    // answer is in user_state
    MumbleProto::RequestBlob msg;
    msg.add_session_texture (id);
    send (msg);
  }

  void Client::channel_description (uint32_t id)
  {
    MumbleProto::RequestBlob msg;
    msg.add_channel_description (id);
    send (msg);
  }

  void Client::comment (uint32_t id)
  {
    MumbleProto::RequestBlob msg;
    msg.add_session_comment (id);
    send (msg);
  }

  template <class T>
  inline T* find_model(std::map<uint32_t, T> map, const std::string &name)
  {
    for (auto &it : map)
      {
        if (it.second.name () == name)
          {
            return &it.second;
          }
      }
    return nullptr;
  }

  User* Client::find_user (const std::string &name)
  {
    return find_model (pimpl->m_users, name);
  }

  Channel* Client::find_channel (const std::string &name)
  {
    return find_model (pimpl->m_channels, name);
  }

  void Client::on_connected (std::function<void()> f)
  {
    pimpl->m_connected_callbacks.push_back (f);
  }

  void Client::on (int type,
                   std::function<void(const ::google::protobuf::Message&)> f)
  {
    pimpl->on (type, f);
  }

  void Client::send (int type, const ::google::protobuf::Message& msg)
  {
    pimpl->send (type, msg);
  }

  int Client::pingtime () const
  {
    return pimpl->m_pingtime;
  }

  const MumbleProto::Reject& Client::rejectmessage () const
  {
    if (pimpl->m_rejectmessage == nullptr)
      {
        throw std::runtime_error ("Reject message not available");
      }
    return *pimpl->m_rejectmessage;
  }

  const std::map<uint32_t, User>& Client::users () const
  {
    return pimpl->m_users;
  }

  const std::map<uint32_t, Channel>& Client::channels () const
  {
    return pimpl->m_channels;
  }

  int Client::max_bandwidth () const
  {
    return pimpl->m_max_bandwidth;
  }

  void Client::register_self ()
  {
    MumbleProto::UserState msg;
    msg.set_session (pimpl->m_session);
    msg.set_user_id (0);
    send (msg);
  }

  bool Client::ready () const
  {
    return pimpl->m_ready;
  }
}
