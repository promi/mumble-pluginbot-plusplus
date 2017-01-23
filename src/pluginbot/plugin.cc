/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 Natenom
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
#include <pluginbot/plugin.hh>

namespace MumblePluginBot
{
  struct Plugin::Impl
  {
    Settings &settings;
    Mumble::Client &client;
    Mumble::AudioPlayer &player;
    uint32_t user_id;

    inline Impl (Settings &settings, Mumble::Client &client,
                 Mumble::AudioPlayer &player,
                 uint32_t user_id)
      : settings (settings), client (client), player (player), user_id (user_id)
    {
    }
  };

  Plugin::Plugin (const Aither::Log &log, Settings &settings, Mumble::Client &cli,
                  Mumble::AudioPlayer &player)
    : pimpl (std::make_unique<Impl> (settings, cli, player, 0)), m_log (log)
  {
  }

  Plugin::~Plugin ()
  {
  }

  std::string Plugin::name ()
  {
    return internal_name ();
  }

  void Plugin::internal_init ()
  {
  }

  void Plugin::init ()
  {
    internal_init ();
  }

  void Plugin::tick (const std::chrono::time_point<std::chrono::system_clock>
                     &time_point)
  {
    (void) time_point;
  }

  void Plugin::chat (const MumbleProto::TextMessage &msg,
                     const std::string &command,
                     const std::string &arguments)
  {
    pimpl->user_id = msg.actor ();
    internal_chat (msg, command, arguments);
  }

  std::string Plugin::internal_help ()
  {
    return "no help text available for plugin: " + name ();
  }

  std::string Plugin::help ()
  {
    return internal_help ();
  }

  void Plugin::message_to (uint32_t user_id, const std::string &message)
  {
    try
      {
        pimpl->client.text_user (user_id, message);
      }
    catch (std::runtime_error &e)
      {
        AITHER_DEBUG ("failed to send message to user: " << user_id);
      }
  }

  void Plugin::private_message (const std::string &message)
  {
    message_to (pimpl->user_id, message);
  }

  void Plugin::channel_message (const std::string &message)
  {
    pimpl->client.text_channel (pimpl->client.me ().channel_id (), message);
  }

  Settings& Plugin::settings ()
  {
    return pimpl->settings;
  }

  Mumble::Client& Plugin::client ()
  {
    return pimpl->client;
  }

  Mumble::AudioPlayer& Plugin::player ()
  {
    return pimpl->player;
  }
}
