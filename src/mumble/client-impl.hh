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

#include "mumble/client.hh"

namespace Mumble
{
  struct Client::Impl
  {
    const Aither::Log &m_log;
    const Configuration &m_config;
    const Certificate &m_cert;
    const std::string m_client_identification;
    std::map<uint32_t, User> m_users;
    std::map<uint32_t, Channel> m_channels;
    bool m_ready = false;
    Codec m_codec = Codec::opus;
    bool m_codec_usable = false;
    int m_max_bandwidth = 0;
    std::unique_ptr<MumbleProto::Reject> m_rejectmessage = nullptr;
    int m_pingtime = 0;
    bool m_connected = false;
    bool m_synced = false;
    std::unique_ptr<Connection> m_connection;
    std::map<int,
        std::list<
        std::function<void(const ::google::protobuf::Message&)>>> m_callbacks;
    std::list<std::function<void()>> m_connected_callbacks;
    std::thread m_read_thread;
    std::thread m_ping_thread;
    bool m_read_thread_running;
    bool m_ping_thread_running;
    uint32_t m_session = 0;

    Impl (const Aither::Log &log, const Configuration &config,
          const Certificate &cert, const std::string &client_identification);
    ~Impl ();
    bool connect ();
    void disconnect ();
    User& text_user_internal (uint32_t session_id, const std::string &message_text);
    User& text_user_img (uint32_t session_id, const FileSystem::path &file);
    Channel& text_channel_internal (uint32_t channel_id,
                                    const std::string &message_text);
    Channel& text_channel_img (uint32_t channel_id, const FileSystem::path &file);
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
    void read ();
    void ping ();
    void run_callbacks (int type, const ::google::protobuf::Message& msg);
    void init_callbacks ();
    void version_exchange ();
    void authenticate ();
    void codec_version (const MumbleProto::CodecVersion &message);
    static uint32_t encode_version(int major, int minor, int patch);
  };
}
