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
#pragma once

#include <chrono>
#include <string>
#include <memory>

#include "mumble/client.hh"
#include "pluginbot/settings.hh"

namespace MumblePluginBot
{
  class Plugin
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  protected:
    void message_to (uint32_t user_id, const std::string &message);
    void channel_message (const std::string &message);
    void private_message (const std::string &message);
  public:
    Plugin ();
    virtual ~Plugin ();
    virtual std::string name () = 0;
    virtual void init (Settings &settings, Mumble::Client &cli);
    virtual void ticks (std::chrono::time_point<std::chrono::system_clock>
                        time_point);
    virtual void handle_chat (const MumbleProto::TextMessage &msg,
                              const std::string &command,
                              const std::string &arguments);
    virtual std::string help ();
  };
}
