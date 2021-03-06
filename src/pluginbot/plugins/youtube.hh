/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 Natenom
    Copyright (c) 2015 netinetwalker
    Copyright (c) 2015 loscoala
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

#include <string>
#include <memory>

#include "pluginbot/plugin.hh"
#include "pluginbot/plugins/messages.hh"

namespace MumblePluginBot
{
  class YoutubePlugin : public Plugin
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  protected:
    std::string internal_name () override;
    void internal_init () override;
    void internal_chat (const MumbleProto::TextMessage &msg,
                        const std::string &command,
                        const std::string &arguments) override;
    std::string internal_help () override;
  public:
    YoutubePlugin (const Aither::Log &log, Settings &settings, Mumble::Client &cli,
                   Mumble::AudioPlayer &player);
    ~YoutubePlugin () override;
  };
}
