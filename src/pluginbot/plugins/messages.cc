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
#include "pluginbot/plugins/messages.hh"

#include <algorithm>
#include <sstream>

#include "pluginbot/html.hh"
#include "mpd/flag-set.hh"

namespace MumblePluginBot
{
  struct MessagesPlugin::Impl
  {
    std::map<uint32_t, FlagSet<MessageType>> priv_notify;
    FlagSet<MessageType> get_message_types (const std::string &arguments);
    std::map<MessageType, std::string> message_type_names;
    std::map<std::string, MessageType> name_message_types;

    inline Impl ()
    {
      message_type_names =
      {
        {MessageType::Volume, "volume"},
        {MessageType::UpdatingDB, "update"},
        {MessageType::Random, "random"},
        {MessageType::Single, "single"},
        {MessageType::XFade, "xfade"},
        {MessageType::Repeat, "repeat"},
        {MessageType::State, "state"}
      };
      for (auto pair : message_type_names)
        {
          name_message_types[pair.second] = pair.first;
        }
    }
  };

  void MessagesPlugin::internal_init ()
  {

  }

  MessagesPlugin::MessagesPlugin (const Aither::Log &log, Settings &settings,
                                  Mumble::Client &cli, Mumble::AudioPlayer &player)
    : Plugin (log, settings, cli, player), pimpl (std::make_unique<Impl> ())
  {

  }

  MessagesPlugin::~MessagesPlugin ()
  {

  }

  std::string MessagesPlugin::name ()
  {
    return "Messages";
  }

  std::string MessagesPlugin::help ()
  {
    const auto &controlstring = settings ().controlstring;
    std::stringstream h;
    h << hr_tag << red_span ("Plugin " + name ()) << br_tag;
    h << b_tag (controlstring + "+ #(" + i_tag ("Hashtag") + ")") <<
      " - Subscribe to a notification." << br_tag;
    h << b_tag (controlstring + "- #(" + i_tag ("Hashtag") + ")") <<
      " - Unsubscribe from a notification." << br_tag;
    h << "You can choose one or more of the following values:" << br_tag;
    h << "volume, random, update, single, xfade, consume, repeat, state" << br_tag;
    h << b_tag (controlstring + "*") << "- List subscribed notifications.";
    h << br_tag << b_tag ("Example:") <<
      " To get a message when the repeat mode changes send the command \"" <<
      controlstring << "+ #repeat\"";
    return h.str ();
  }

  void MessagesPlugin::send_message (const std::string message, MessageType type)
  {
    if (settings ().chan_notify.test (type))
      {
        for (auto pn : pimpl->priv_notify)
          {
            if (pn.second.test (type))
              {
                try
                  {
                    message_to (pn.first, message);
                  }
                catch (...)
                  {

                  }
              }
          }
      }
  }

  FlagSet<MessageType> MessagesPlugin::Impl::get_message_types (
    const std::string &arguments)
  {
    FlagSet<MessageType> flag_set;
    std::stringstream ss {arguments};
    for (std::string argument; std::getline (ss, argument, ' '); )
      {
        if (argument.substr (0, 1) == "#")
          {
            auto flag_pair = name_message_types.find (argument.substr (1));
            if (flag_pair != name_message_types.end ())
              {
                flag_set.set (flag_pair->second);
              }
          }
      }
    return flag_set;
  }

  void MessagesPlugin::handle_chat (const MumbleProto::TextMessage &msg,
                                    const std::string &command,
                                    const std::string &arguments)
  {
    auto &flag_set = pimpl->priv_notify[msg.actor ()];
    if (command == "+")
      {
        flag_set |= pimpl->get_message_types (arguments);
      }
    else if (command == "-")
      {
        flag_set &= ~pimpl->get_message_types (arguments);
      }
    else if (command == "*")
      {
        std::vector<std::string> names;
        for (auto &pair : pimpl->message_type_names)
          {
            if (flag_set.test (pair.first))
              {
                names.push_back (pair.second);
              }
          }
        std::sort (names.begin (), names.end ());
        std::stringstream send;
        for (auto &name : names)
          {
            send << " #" << name;
          }
        std::string s = send.str ();
        if (s != "")
          {
            s = "You listen to the following MPD-Channels:" + s + ".";
          }
        else
          {
            s  = "You do not listen to any MPD-Channels";
          }
        message_to (msg.actor (), s);
      }
  }
}
