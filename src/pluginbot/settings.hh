/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 Natenom
    Copyright (c) 2015 Stunner1984
    Copyright (c) 2015 dafoxia
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

#include <chrono>
#include <string>
#include <map>

#include "filesystem/filesystem.hh"
#include "mpd/flag-set.hh"

namespace MumblePluginBot
{
  // Send Message when ...
  enum class MessageType : uint8_t
  {
    Volume = 0x01,
    UpdatingDB = 0x02,
    Random = 0x04,
    Single = 0x08,
    XFade = 0x10,
    Consume = 0x20,
    Repeat = 0x40,
    State = 0x80
  };

  class Settings
  {
  public:
    Settings ();

    std::string version {"0.1"};
    FileSystem::path main_tempdir {"temp"};
    bool ducking = false;
    uint ducking_vol = 20;
    bool control_automute = true;
    FlagSet<MessageType> chan_notify;
    std::string controlstring = {"."};
    bool debug = true;
    bool verbose = true;
    std::chrono::duration<double, std::ratio<1>> tick_duration { 1 };
    bool listen_to_private_message_only = false;
    bool listen_to_registered_users_only = true;
    bool use_vbr = true;
    bool stop_on_unregistered_users = true;
    bool use_comment_for_status_display = true;
    // Blacklist entry: user_hash=username
    std::map<std::string, std::string> blacklist;
    struct
    {
      std::string host {"127.0.0.1"};
      uint16_t port = 64738;
      std::string username {"MumblePluginbotPlusPlus"};
      std::string userpassword {""};
      std::string targetchannel {""};
    } connection;
    // TODO: This really belongs to libmumble-pluginbot-plusplus-mumble!
    struct
    {
      bool allow_html = false;
      uint32_t message_length = 0;
      uint32_t image_message_length = 0;
    } server_config;
    // TODO: This really belongs to libmumble-pluginbot-plusplus-mumble!
    struct
    {
      std::string version;
      bool positional = false;
      bool push_to_talk = false;
    } suggest_config;
    std::string logo;
    std::string superanswer;
    struct
    {
      FileSystem::path fifopath {"mpd.fifo"};
      std::string host {"localhost"};
      uint16_t port = 7701;
      FileSystem::path musicdir {"mpd/music"};
    } mpd;
    FileSystem::path certdir {"certs"};
    uint quality_bitrate = 72000;
    uint initial_volume = 65;
    bool controllable = true;
    bool need_binding = false;
    std::string boundto;
    // TODO: Generate random password on startup?
    std::string superpassword;
    struct
    {
      std::string download_subdir = "downloadedfromyt";
      std::string temp_subdir = "youtubeplugin";
      std::string youtubedl = "youtube-dl";
      bool stream = false;
      bool convert_to_mp3 = false;
      uint max_results = 200;
    } youtube;
    /*
    soundcloud_downloadsubdir", "downloadedfromsc"},
    soundcloud_tempsubdir", "soundcloudplugin"},
    soundcloud_youtubedl", "youtube-dl"},
    soundcloud_to_mp3", "nil"},
    soundcloud_youtubedl_options", ""},
    soundcloud_commandlineprefixes", ""},
    bandcamp_downloadsubdir", "downloadedfrombc"},
    bandcamp_tempsubdir", "bandcampplugin"},
    bandcamp_youtubedl", "youtube-dl"},
    bandcamp_to_mp3", "nil"},
    bandcamp_youtubedl_options", ""},
    bandcamp_commandlineprefixes", ""},
    ektoplazm_downloadsubdir", "ektoplazm"},
    ektoplazm_tempsubdir", "ektoplazmplugin"},
    mpd_musicfolder", "music"},
    control_historysize", "20"},
      {
        "mpd_template_comment_disabled",
        "<b>Artist: </b>DISABLED<br />"
        "<b>Title: </b>DISABLED<br />"
        "<b>Album: </b>DISABLED<br /><br />"
        "<b>Write %shelp to me, to get a list of my commands!"
      },
      {
        "mpd_template_comment_enabled",
        "<b>Artist: </b>%s<br />"
        "<b>Title: </b>%s<br />" \
        "<b>Album: </b>%s<br /><br />" \
        "<b>Write %shelp to me, to get a list of my commands!</b>"
      }
    };
    */
  };
}
