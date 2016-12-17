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
#include "pluginbot/plugins/version.hh"

#include <sstream>

#include "git-info.hh"
#include "pluginbot/html.hh"

namespace MumblePluginBot
{
  std::string VersionPlugin::name ()
  {
    return "version";
  }

  std::string VersionPlugin::help ()
  {
    std::stringstream h;
    h << hr_tag;
    h << red_span ("Plugin " + name ()) << br_tag;
    h << b_tag (settings ().controlstring + "version") << br_tag;
    return h.str ();
  }

  void VersionPlugin::handle_chat (const MumbleProto::TextMessage &msg,
                                   const std::string &command,
                                   const std::string &arguments)
  {
    Plugin::handle_chat (msg, command, arguments);
    if (command == "version")
      {
        private_message (std::string ("Version: ") + GIT_DESCRIBE);
      }
  }
}
