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
#include "mumble/cert-manager.hh"
#include "mumble/mumble-2-mumble.hh"
#include "mumble/audio-recorder.hh"
#include "mumble/configuration.hh"
#include "mumble/messages.hh"
#include "mumble/audio-player.hh"
#include "mumble/connection.hh"
#include "mumble/channel.hh"
#include "mumble/user.hh"
#include "mumble/codec.hh"

namespace Mumble
{
  class Client
  {
  private:
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
    uint m_bitrate = 0;
    std::unique_ptr<Mumble2Mumble> m_m2m;
    std::unique_ptr<AudioRecorder> m_recorder;
    std::unique_ptr<Connection> m_conn;
    std::map<int,
        std::list<
        std::function<void(const ::google::protobuf::Message&)>>> m_callbacks;
    std::list<std::function<void()>> m_connected_callbacks;
    std::unique_ptr<AudioPlayer> m_audio_streamer;
    std::thread m_read_thread;
    std::thread m_ping_thread;
    bool m_read_thread_running;
    bool m_ping_thread_running;
    uint32_t m_session;
  public:
    Client (const Aither::Log &log, const Configuration &config,
            const Certificate &cert,
            const std::string &client_identification = "Unknown 0.1");
    inline ~Client ()
    {
      m_read_thread_running = false;
      m_ping_thread_running = false;
      m_read_thread.join ();
      m_ping_thread.join ();
    }
    bool connect ();
    void disconnect ();
    inline bool connected () const
    {
      return m_connected;
    }
    inline bool synced () const
    {
      return m_synced;
    }
    const CertManager& cert_manager ();
    const AudioRecorder& recorder ();
    inline std::string codec () const
    {
      if (!m_codec_usable)
        {
          throw std::string ("no usable codec");
        }
      return to_string (m_codec);
    }
    inline Codec codec_int () const
    {
      return m_codec;
    }
    void mumble2mumble (bool rec);
    std::list<uint32_t> m2m_speakers ();
    std::vector<int16_t> m2m_get_frame (uint32_t speaker);
    void m2m_writeframe (const std::vector<int16_t> &frame);
    size_t m2m_getsize (uint32_t speaker);
    void deaf (bool b = true);
    void mute (bool b = true);
    AudioPlayer& player ();
    User& me ();
    std::string imgmsg (const FileSystem::path &file);
    void comment (const std::string &newcomment);
    Channel& join_channel (const std::string &channel);
    template <class T>
    inline User& text_user (const T &user, const std::string &message)
    {
      return text_user_internal (user_session (user), message);
    }
    template<class T>
    inline User& text_user_img (const T &user, const FileSystem::path &file)
    {
      return text_user_img_internal (user_session (user), file);
    }
    template<class T>
    inline Channel& text_channel (const T &channel, const std::string &message)
    {
      return text_channel_internal (channel_id (channel), message);
    }
    template<class T>
    inline Channel& text_channel_img (const T &channel,
                                      const FileSystem::path &file)
    {
      return text_channel_img_internal (channel_id (channel),  file);
    }
    void avatar (const std::string &img);
    void fetch_avatar (uint32_t id);
    void fetch_channel_description (uint32_t id);
    void fetch_session_comment (uint32_t id);
    User* find_user (const std::string &name);
    Channel* find_channel (const std::string &name);
    void bitrate (int bitrate);
    int bitrate ();
    void frame_length (std::chrono::milliseconds framelength);
    std::chrono::milliseconds frame_length ();
    inline void on_connected (std::function<void()> f)
    {
      m_connected_callbacks.push_back (f);
    }
    /*
    // Doesn't work like this ...
    template <class T>
    inline void remove_callback (std::function<void(const T&)> f)
    {
      // m_callbacks [std::type_index (typeid (T))].remove (f);
    }
    */
    template <class T>
    inline void on (std::function<void(const T&)> f)
    {
      m_callbacks [Messages::sym_to_type.at (std::type_index (typeid (
          T)))].push_back ([f] (const auto &message)
      {
        f (dynamic_cast<const T&>(message));
      });
    }
    template <class T>
    inline void send (const T &message)
    {
      m_conn->send_message (Messages::sym_to_type.at (std::type_index (typeid (T))),
                            message);
    }
    inline auto& pingtime () const
    {
      return m_pingtime;
    }
    inline const auto& rejectmessage () const
    {
      return m_rejectmessage;
    }
    inline auto& users ()
    {
      return m_users;
    }
    inline auto& channels ()
    {
      return m_channels;
    }
    inline auto& max_bandwidth () const
    {
      return m_max_bandwidth;
    }
    void register_self ();
    inline bool ready ()
    {
      return m_ready;
    }
  private:
    User& text_user_internal (uint32_t session_id, const std::string &message_text);
    User& text_user_img (uint32_t session_id, const FileSystem::path &file);
    Channel& text_channel_internal (uint32_t channel_id,
                                    const std::string &message_text);
    Channel& text_channel_img (uint32_t channel_id, const FileSystem::path &file);
    void send_internal (int type, const ::google::protobuf::Message& msg);
    void read ();
    void ping ();
    void run_callbacks (int type, const ::google::protobuf::Message& msg);
    void init_callbacks ();
    void version_exchange ();
    void authenticate ();
    void codec_version (const MumbleProto::CodecVersion &message);
    uint32_t channel_id (const std::string &channelname);
    uint32_t channel_id (const Channel &channel);
    uint32_t channel_id (uint32_t user_id);
    uint32_t user_session (const std::string &username);
    uint32_t user_session (const User &user);
    uint32_t user_session (uint32_t user_id);
    static uint32_t encode_version(int major, int minor, int patch)
    {
      return (major << 16) | (minor << 8) | (patch & 0xff);
    }
  };
}
