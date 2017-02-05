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
#include <iostream>
#include <openssl/sha.h>

#include "mumble/mumble.hh"
#include "mumble/client.hh"
#include "mumble/img-reader.hh"
#include "mumble/version.hh"

namespace Mumble
{
  Client::Client (const Aither::Log &log, const std::string &host, int port,
                  const std::string &username, const std::string password,
                  std::function <void(Mumble::Configuration&)> conf_func)
    : m_log (log)
  {
    m_config = Mumble::configuration;
    m_config.host = host;
    m_config.port = port;
    m_config.username = username;
    m_config.password = password;
    if (conf_func)
      {
        conf_func (m_config);
      }
  }

  bool Client::connect ()
  {
    m_conn = std::make_unique<Connection> (m_config.host, m_config.port,
                                           cert_manager ());
    m_conn->connect ();
    if (!m_conn->connected ())
      {
        return false;
      }
    init_callbacks ();
    version_exchange ();
    authenticate ();
    m_connected = true;
    m_read_thread_running = true;
    m_read_thread = std::thread ([this] { read (); });
    m_ping_thread_running = true;
    m_ping_thread = std::thread ([this] { ping (); });
    return m_connected;
  }

  void Client::disconnect ()
  {
    m_conn->disconnect ();
    m_synced = false;
    m_ready = false;
    m_connected = false;
    m_audio_streamer = nullptr;
    m_m2m = nullptr;
    m_read_thread_running = false;
    m_read_thread.join ();
    m_ping_thread_running = false;
    m_ping_thread.join ();
  }

  const CertManager& Client::cert_manager ()
  {
    if (m_cert_manager == nullptr)
      {
        m_cert_manager = std::make_unique<CertManager> (m_config.username,
                         m_config.ssl_cert_opts);
      }
    return *m_cert_manager;
  }

  const AudioRecorder& Client::recorder ()
  {
    if (!m_codec_usable)
      {
        throw std::runtime_error ("no usable codec");
      }
    if (m_recorder == nullptr)
      {
        Mumble::ClientIntf intf;
        m_recorder = std::make_unique<AudioRecorder> (intf,
                     m_config.sample_rate);
      }
    return *m_recorder;
  }

  void Client::mumble2mumble (bool rec)
  {
    if (m_m2m != nullptr)
      {
        return;
      }
    m_m2m = std::make_unique<Mumble2Mumble> (m_log, m_codec, *m_conn,
            m_config.sample_rate, m_config.sample_rate / 100, 1, m_config.bitrate);
    if (rec)
      {
        on<MumbleProto::UDPTunnel> ([this] (auto m)
        {
          m_m2m->process_udp_tunnel (m);
        });
      }
  }

  std::list<uint32_t> Client::m2m_speakers ()
  {
    if (m_m2m == nullptr)
      {
        throw std::runtime_error ("M2M not initializied");
      }
    return m_m2m->getspeakers ();
  }

  std::vector<int16_t> Client::m2m_get_frame (uint32_t speaker)
  {
    if (m_m2m == nullptr)
      {
        throw std::runtime_error ("M2M not initializied");
      }
    return m_m2m->getframe (speaker);
  }

  void Client::m2m_writeframe (const std::vector<int16_t>& frame)
  {
    if (m_m2m == nullptr)
      {
        throw std::runtime_error ("M2M not initializied");
      }
    return m_m2m->produce (frame);
  }

  size_t Client::m2m_getsize (uint32_t speaker)
  {
    if (m_m2m == nullptr)
      {
        throw std::runtime_error ("M2M not initializied");
      }
    return m_m2m->getsize (speaker);
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

  AudioPlayer& Client::player ()
  {
    if (!m_codec_usable)
      {
        throw std::runtime_error ("no usable codec");
      }
    if (!m_audio_streamer)
      {
        m_audio_streamer = std::make_unique<AudioPlayer> (m_log, m_codec, *m_conn,
                           m_config.sample_rate, m_config.bitrate);
      }
    return *m_audio_streamer;
  }

  User& Client::me ()
  {
    return m_users.at (m_session);
  }

  std::string Client::imgmsg (const std::string &file)
  {
    return ImgReader::msg_from_file (file);
  }

  void Client::comment (const std::string &newcomment)
  {
    MumbleProto::UserState message;
    message.set_comment (newcomment);
    send (message);
  }

  Channel& Client::join_channel(const std::string &channel)
  {
    uint32_t id = channel_id (channel);
    MumbleProto::UserState msg;
    msg.set_session (m_session);
    msg.set_channel_id (id);
    send (msg);
    return m_channels.at (id);
  }

  User& Client::text_user_internal (uint32_t session_id,
                                    const std::string &message_text)
  {
    MumbleProto::TextMessage msg;
    msg.add_session (session_id);
    msg.set_message (message_text);
    send (msg);
    return m_users.at (session_id);
  }

  User& Client::text_user_img (uint32_t session_id, const std::string &file)
  {
    return text_user_internal (session_id, ImgReader::msg_from_file (file));
  }

  Channel& Client::text_channel_internal (uint32_t channel_id,
                                          const std::string &message_text)
  {
    MumbleProto::TextMessage msg;
    msg.add_channel_id (channel_id);
    msg.set_message (message_text);
    send (msg);
    return m_channels.at (channel_id);
  }

  Channel& Client::text_channel_img (uint32_t channel_id, const std::string &file)
  {
    return text_channel_internal (channel_id, ImgReader::msg_from_file (file));
  }

  void Client::register_self ()
  {
    MumbleProto::UserState msg;
    msg.set_session (m_session);
    msg.set_user_id (0);
    send (msg);
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

  void Client::fetch_avatar (uint32_t id)
  {
    // send a request for avatar image
    // answer is in user_state
    MumbleProto::RequestBlob msg;
    msg.add_session_texture (id);
    send (msg);
  }

  void Client::fetch_channel_description (uint32_t id)
  {
    MumbleProto::RequestBlob msg;
    msg.add_channel_description (id);
    send (msg);
  }

  void Client::fetch_session_comment (uint32_t id)
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
    return find_model (m_users, name);
  }

  Channel* Client::find_channel (const std::string &name)
  {
    return find_model (m_channels, name);
  }

  void Client::bitrate (int bitrate)
  {
    try
      {
        m_audio_streamer->bitrate (bitrate);
      }
    catch (...)
      {
      }
  }

  int Client::bitrate ()
  {
    try
      {
        return m_audio_streamer->bitrate ();
      }
    catch (...)
      {
        return 0;
      }
  }

  void Client::frame_length (std::chrono::milliseconds framelength)
  {
    try
      {
        m_audio_streamer->framelength (framelength);
      }
    catch (...)
      {
      }
  }

  std::chrono::milliseconds Client::frame_length ()
  {
    using namespace std::chrono_literals;
    try
      {
        return m_audio_streamer->framelength ();
      }
    catch (...)
      {
        return 0ms;
      }
  }

  void Client::read ()
  {
    while (m_read_thread_running)
      {
        try
          {
            auto pair = m_conn->read_message ();
            run_callbacks (pair.first, *pair.second);
          }
        catch (const std::string &s)
          {
            std::cerr << "Could not read message from server: " << s << "\n";
          }
      }
  }

  void Client::ping ()
  {
    using namespace std::chrono_literals;
    using namespace std::chrono;

    while (m_ping_thread_running)
      {
        MumbleProto::Ping msg;
        msg.set_timestamp (duration_cast<milliseconds>
                           (system_clock::now ().time_since_epoch ()).count ());
        send (msg);
        std::this_thread::sleep_for (15s);
      }
  }

  void Client::run_callbacks (int type, const ::google::protobuf::Message& msg)
  {
    auto cbs = m_callbacks.find (type);
    if (cbs != std::end (m_callbacks))
      {
        for (auto f : cbs->second)
          {
            f (msg);
          }
      }
  }

  template<class T>
  constexpr const T& clamp (const T& v, const T& lo, const T& hi)
  {
    return v < lo ? lo : v > hi ? hi : v;
  }

  void Client::init_callbacks ()
  {
    on<MumbleProto::ServerSync> ([this] (auto msg)
    {
      if (msg.has_max_bandwidth ())
        {
          m_max_bandwidth = msg.max_bandwidth ();
          m_config.bitrate = clamp (m_config.bitrate, 0, m_max_bandwidth - 32000);
        }
      m_session = msg.session ();
      m_synced = true;
      for (auto f : m_connected_callbacks)
        {
          f ();
        }
    });
    on<MumbleProto::ChannelState> ([this] (auto msg)
    {
      auto channel_id = msg.channel_id ();
      auto channel = m_channels.find (channel_id);
      if (channel == std::end (m_channels))
        {
          m_channels.emplace (channel_id, msg);
        }
      else
        {
          channel->second.update (msg);
        }
    });
    on<MumbleProto::ChannelRemove> ([this] (auto msg)
    {
      m_channels.erase (msg.channel_id ());
    });
    on<MumbleProto::UserState> ([this] (auto msg)
    {
      auto session = msg.session ();
      auto user = m_users.find (session);
      if (user == std::end (m_users))
        {
          m_users.emplace (session, msg);
        }
      else
        {
          user->second.update (msg);
        }
    });
    on<MumbleProto::UserRemove> ([this] (auto msg)
    {
      auto session = msg.session ();
      if (session == m_session)
        {
          this->disconnect ();
        }
      m_users.erase (session);
    });
    on<MumbleProto::CodecVersion> ([this] (auto msg)
    {
      this->codec_version (msg);
    });
    on<MumbleProto::Ping> ([this] (auto msg)
    {
      using namespace std::chrono;
      using namespace std::chrono_literals;
      m_pingtime = (duration_cast<milliseconds>
                    (system_clock::now ().time_since_epoch ()).count ()) - msg.timestamp ();
      m_ready = true;
    });
    on<MumbleProto::CryptSetup> ([this] (auto msg)
    {
      // For later implementation of UDP communication
      (void) msg;
    });
    on<MumbleProto::Reject> ([this] (auto msg)
    {
      m_rejectmessage = std::make_unique<MumbleProto::Reject> (msg);
      this->disconnect ();
    });
    on<MumbleProto::ServerConfig> ([this] (auto msg)
    {
      (void) msg;
    });
  }

  void Client::version_exchange ()
  {
    MumbleProto::Version msg;
    msg.set_version (encode_version (1, 3, 0));
    msg.set_release (std::string ("mumble-pluginbot-plusplus ") + Mumble::_VERSION);
    msg.set_os ("Unknown");
    msg.set_os_version ("Unknown");
    // os: %x{uname -s -m}.strip,
    // os_version: %x{uname -v}.strip
    send (msg);
  }

  void Client::authenticate ()
  {
    // encoder = Celt::Encoder.new 32000, 180, 1, 1
    MumbleProto::Authenticate msg;
    msg.set_username (m_config.username);
    msg.set_password (m_config.password);
    // celt_versions: [encoder.bitstream_version],
    msg.set_opus (true);
    send (msg);
  }

  void Client::codec_version (const MumbleProto::CodecVersion &message)
  {
    if (message.has_opus ())
      {
        m_codec = Codec::opus;
        m_codec_usable = true;
      }
    else
      {
        m_codec_usable = false;
        /*
              encoder = Celt::Encoder.new 32000, 180, 1, 1
              @codec =  [Codec::alpha, Codec::beta][[message.alpha, message.beta].index(encoder.bitstream_version)]
              encoder.destroy
        */
      }
    if (m_m2m != nullptr)
      {
        m_m2m->codec (m_codec);
      }
    if (m_audio_streamer != nullptr)
      {
        m_audio_streamer->codec (m_codec);
      }
  }

  uint32_t Client::channel_id (const std::string &channelname)
  {
    Channel* channel = find_channel (channelname);
    if (channel == nullptr)
      {
        throw std::runtime_error ("ChannelNotFound");
      }
    return channel_id (*channel);
  }

  uint32_t Client::channel_id (const Channel &channel)
  {
    return channel_id (channel.channel_id ());
  }

  uint32_t Client::channel_id (uint32_t channel_id)
  {
    const auto &it = m_channels.find (channel_id);
    if (it == std::end (m_channels))
      {
        throw std::runtime_error ("ChannelNotFound");
      }
    return channel_id;
  }

  uint32_t Client::user_session (const std::string &username)
  {
    User* user = find_user (username);
    if (user == nullptr)
      {
        throw std::runtime_error ("UserNotFound");
      }
    return user_session (*user);
  }

  uint32_t Client::user_session (const User &user)
  {
    return user_session (user.session ());
  }

  uint32_t Client::user_session (uint32_t session)
  {
    if (m_users.find (session) == std::end (m_users))
      {
        throw std::runtime_error ("UserNotFound");
      }
    return session;
  }
}
