/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 Natenom
    Copyright (c) 2015 Stunner1984
    Copyright (c) 2015 dafoxia
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

#include <string>
#include <map>
#include <experimental/filesystem>

namespace MumblePluginBot
{
  class Settings
  {
  public:
    Settings ();

    std::string version {"0.1"};
    std::experimental::filesystem::path main_tempdir {"temp"};
    bool ducking = false;
    uint ducking_vol = 20;
    bool control_automute = true;
    uint16_t chan_notify = 0x0000;
    std::string controlstring = {"."};
    bool debug = true;
    bool verbose = true;
    std::chrono::duration<double, std::ratio<1>> tick_sleep_duration { 0.5 };
    bool listen_to_private_message_only = false;
    bool listen_to_registered_users_only = true;
    bool use_vbr = true;
    bool stop_on_unregistered_users = true;
    bool use_comment_for_status_display = true;
    bool set_comment_available = false;
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
      std::experimental::filesystem::path fifopath {"mpd.fifo"};
      std::string host {"localhost"};
      uint16_t port = 7701;
    } mpd;
    std::experimental::filesystem::path certdir {"certs"};
    uint quality_bitrate = 72000;
    uint initial_volume = 65;
    bool controllable = true;
    bool need_binding = false;
    std::string boundto;
    // TODO: Generate random password on startup?
    std::string superpassword;
    /*
    youtube_downloadsubdir", "downloadedfromyt"},
    youtube_tempsubdir", "youtubeplugin"},
    youtube_stream", "nil"},
    youtube_youtubedl", "youtube-dl"},
    youtube_to_mp3", "nil"},
    youtube_youtubedl_options", ""},
    youtube_commandlineprefixes", ""},
    youtube_maxresults", "200"},
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
