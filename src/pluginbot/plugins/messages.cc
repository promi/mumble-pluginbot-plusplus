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

#include <sstream>

#include "pluginbot/html.hh"
#include "mpd/flag-set.hh"

namespace MumblePluginBot
{
  struct MessagesPlugin::Impl
  {
    std::map<uint32_t, FlagSet<MessageType>> priv_notify;
  };

  void MessagesPlugin::internal_init ()
  {

  }

  MessagesPlugin::MessagesPlugin ()
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

  void MessagesPlugin::handle_chat (const MumbleProto::TextMessage &msg,
                                    const std::string &command,
                                    const std::string &arguments)
  {
    /*
    super
    @priv_notify[msg.actor] = 0 if @priv_notify[msg.actor].nil?
    if message[2] == '#'
      message.split.each do |command|
        case command
        when "#volume"
          add = Cvolume
        when "#update"
          add = Cupdating_db
        when "#random"
          add = Crandom
        when "#single"
          add = Csingle
        when "#xfade"
          add = Cxfade
        when "#consume"
          add = Cconsume
        when "#repeat"
          add = Crepeat
        when "#state"
          add = Cstate
        else
          add = 0
        end
        @priv_notify[msg.actor] |= add if message[0] == '+'
        @priv_notify[msg.actor] &= ~add if message[0] == '-'
      end
    end
    if message == '*' && !@priv_notify[msg.actor].nil?
      send = ""
      send << " #volume" if (@priv_notify[msg.actor] & Cvolume) > 0
      send << " #update" if (@priv_notify[msg.actor] & Cupdating_db) > 0
      send << " #random" if (@priv_notify[msg.actor] & Crandom) > 0
      send << " #single" if (@priv_notify[msg.actor] & Csingle) > 0
      send << " #xfade" if (@priv_notify[msg.actor] & Cxfade) > 0
      send << " #repeat" if (@priv_notify[msg.actor] & Crepeat) > 0
      send << " #state" if (@priv_notify[msg.actor] & Cstate) > 0
      if send != ""
        send = "You listen to following MPD-Channels:" + send + "."
      else
        send << "You listen to no MPD-Channels"
      end
      @@bot[:cli].text_user(msg.actor, send)
    end
    */
  }
}
