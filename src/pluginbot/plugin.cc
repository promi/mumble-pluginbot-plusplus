/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 Natenom
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
#include <pluginbot/plugin.hh>

namespace MumblePluginBot
{
  struct Plugin::Impl
  {
    Settings &settings;
    Mumble::Client &cli;
    uint32_t user_id;

    inline Impl (Settings &settings, Mumble::Client &cli, uint32_t user_id)
      : settings (settings), cli (cli), user_id (user_id)
    {
    }
  };

  Plugin::Plugin ()
  {
  }

  Plugin::~Plugin ()
  {
  }

  void Plugin::init (Settings &settings, Mumble::Client &cli)
  {
    pimpl = std::make_unique<Impl> (settings, cli, 0);
  }
  
  void Plugin::ticks (std::chrono::time_point<std::chrono::system_clock>
                      time_point)
  {
    (void) time_point;
  }

  void Plugin::handle_chat (const MumbleProto::TextMessage &msg,
                            const std::string &command,
                            const std::string &arguments)
  {
    pimpl->user_id = msg.actor ();
    (void) command;
    (void) arguments;
  }
  
  /*
    def handle_command(command)
    #raise "#{self.class.name} doesn't implement `handle_command`!"
    end
  */

  std::string Plugin::help ()
  {
    return "no help text available for plugin: " + name ();
  }

  void Plugin::message_to (uint32_t user_id, const std::string &message)
  {
    // TODO: When can this fail and with what exception?
    pimpl->cli.text_user (user_id, message);
  }

  void Plugin::private_message (const std::string &message)
  {
    message_to (pimpl->user_id, message);
  }

  void Plugin::channel_message (const std::string &message)
  {
    pimpl->cli.text_channel (pimpl->cli.me ().channel_id (), message);
  }
}
