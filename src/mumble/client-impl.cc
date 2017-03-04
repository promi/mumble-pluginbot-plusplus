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

namespace Mumble
{
  Client::Impl::Impl (const Aither::Log &log, const Configuration &config,
                      const Certificate &cert, const std::string &client_identification)
    : m_log (log), m_config (config), m_cert (cert),
      m_client_identification (client_identification)
  {
  }

  Client::Impl::~Impl ()
  {
    m_read_thread_running = false;
    m_ping_thread_running = false;
    m_read_thread.join ();
    m_ping_thread.join ();
  }

  bool Client::Impl::connect ()
  {
    m_connection = std::make_unique<Connection> (m_log, m_config.host,
                   m_config.port,
                   m_cert);
    m_connection->connect ();
    if (!m_connection->connected ())
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

  void Client::Impl::disconnect ()
  {
    m_connection->disconnect ();
    m_synced = false;
    m_ready = false;
    m_connected = false;
    m_read_thread_running = false;
    m_read_thread.join ();
    m_ping_thread_running = false;
    m_ping_thread.join ();
  }

  void Client::Impl::on (int type,
                         std::function<void(const ::google::protobuf::Message&)> f)
  {
    // TODO: Use libsigc++ in the future, also enables unregistering!
    m_callbacks[type].push_back (f);
  }

  void Client::Impl::send (int type, const ::google::protobuf::Message& msg)
  {
    m_connection->send_message (type, msg);
  }

  void Client::Impl::read ()
  {
    while (m_read_thread_running)
      {
        try
          {
            auto pair = m_connection->read_message ();
            run_callbacks (pair.first, *pair.second);
          }
        catch (const std::string &s)
          {
            AITHER_ERROR("Could not read message from server: " << s << "\n");
            throw;
          }
      }
  }

  void Client::Impl::ping ()
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

  void Client::Impl::run_callbacks (int type,
                                    const ::google::protobuf::Message& msg)
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

  void Client::Impl::init_callbacks ()
  {
    on<MumbleProto::ServerSync> ([this] (auto msg)
    {
      if (msg.has_max_bandwidth ())
        {
          m_max_bandwidth = msg.max_bandwidth ();
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

  void Client::Impl::version_exchange ()
  {
    MumbleProto::Version msg;
    msg.set_version (encode_version (1, 3, 0));
    msg.set_release (m_client_identification);
    msg.set_os ("Unknown");
    msg.set_os_version ("Unknown");
    // os: %x{uname -s -m}.strip,
    // os_version: %x{uname -v}.strip
    send (msg);
  }

  void Client::Impl::authenticate ()
  {
    // encoder = Celt::Encoder.new 32000, 180, 1, 1
    MumbleProto::Authenticate msg;
    msg.set_username (m_config.username);
    msg.set_password (m_config.password);
    // celt_versions: [encoder.bitstream_version],
    msg.set_opus (true);
    send (msg);
  }

  void Client::Impl::codec_version (const MumbleProto::CodecVersion &message)
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
  }

  uint32_t Client::Impl::encode_version(int major, int minor, int patch)
  {
    return (major << 16) | (minor << 8) | (patch & 0xff);
  }
}
