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
#pragma once

#include <sstream>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <list>
#include <typeinfo>
#include <typeindex>
#include <thread>
#include <google/protobuf/message.h>

#include "aither/log.hh"
#include "filesystem/filesystem.hh"
#include "Mumble.pb.h"
#include "mumble/configuration.hh"
#include "mumble/messages.hh"
#include "mumble/connection.hh"
#include "mumble/channel.hh"
#include "mumble/user.hh"
#include "mumble/codec.hh"

namespace Mumble
{
  class Client
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    Client (const Aither::Log &log, const Configuration &config,
            const Certificate &cert,
            const std::string &client_identification = "Unknown 0.1");
    ~Client ();
    bool connect ();
    void disconnect ();
    bool connected () const;
    Connection& connection () const;
    bool synced () const;
    std::string codec () const;
    Codec codec_int () const;
    void deaf (bool b = true);
    void mute (bool b = true);
    User& me ();
    void comment (const std::string &newcomment);
    void join_channel (const std::string &name);
    void text_user (uint32_t session, const std::string &message);
    void text_user_img (uint32_t session, const FileSystem::path &file);
    void text_channel (uint32_t channel_id, const std::string &message);
    void text_channel_img (uint32_t channel_id, const FileSystem::path &file);
    void avatar (const std::string &img);
    void avatar (uint32_t id);
    void channel_description (uint32_t id);
    void comment (uint32_t id);
    User* find_user (const std::string &name);
    Channel* find_channel (const std::string &name);
    void on_connected (std::function<void()> f);
    std::map<int,
        std::list<
        std::function<void(const ::google::protobuf::Message&)>>>& callbacks ();
    void on (int type, std::function<void(const ::google::protobuf::Message&)> f);
    template <class T>
    inline void on (std::function<void(const T&)> f)
    {
      on (Messages::sym_to_type.at (std::type_index (typeid (T))), [f] (
            const auto &message)
      {
        f (dynamic_cast<const T&>(message));
      });
    }
    void send (int type, const ::google::protobuf::Message& msg);
    template <class T>
    inline void send (const T &msg)
    {
      send (Messages::sym_to_type.at (std::type_index (typeid (T))), msg);
    }
    int pingtime () const;
    const MumbleProto::Reject& rejectmessage () const;
    const std::map<uint32_t, User>& users () const;
    const std::map<uint32_t, Channel>& channels () const;
    int max_bandwidth () const;
    void register_self ();
    bool ready () const;
  };
}
