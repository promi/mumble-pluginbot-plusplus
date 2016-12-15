/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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
#include "pluginbot/setting-descriptors.hh"

#include <stdexcept>

#define BOOL_GETTER(setting) [&] () { return setting ? "true" : "false"; }

#define BOOL_SETTER(setting)                                          \
  [&] (const std::string& value)                                      \
  {                                                                   \
    if (value == "true")                                              \
      {                                                               \
        setting = true;                                               \
      }                                                               \
    else if (value == "false")                                        \
      {                                                               \
        setting = false;                                              \
      }                                                               \
    else                                                              \
      {                                                               \
        throw std::invalid_argument ("'" + value + "' is neither " +  \
                                     "'true' nor 'false'");           \
      }                                                               \
  }

#define TO_STRING_GETTER(setting) [&] () { return std::to_string (setting); }

#define TO_STRING_SUFFIX_GETTER(setting,suffix) [&] () \
  { return std::to_string (setting) + suffix; }

#define UINT_RANGE_SETTER(setting,max)                              \
  [&] (const std::string& value)                                    \
  {                                                                 \
    uint i = (uint) std::stoi (value);                              \
    if (i > max)                                                    \
      {                                                             \
        throw std::out_of_range ("'" + value + "' not in range '" + \
                                 std::to_string (0) + ".." +        \
                                 std::to_string (max) + "'");       \
      }                                                             \
    setting = i;                                                    \
  }

#define DOUBLE_DURATION_GETTER(setting) [&] () \
  { return std::to_string (setting.count ()) + "s"; }

#define DOUBLE_DURATION_SETTER(setting) [&] (const std::string& value)  \
  {                                                                     \
    setting = std::chrono::duration<double, std::ratio<1>> { std::stod (value) }; \
  }

#define STRING_GETTER(setting) [&] () { return setting; }

#define STRING_SETTER(setting) [&] (const std::string& value) { setting = value; }

#define BOOL(setting) BOOL_GETTER(setting), BOOL_SETTER(setting)
#define STRING(setting) STRING_GETTER(setting), STRING_SETTER(setting)
#define PATH(setting) STRING(setting)
#define UINT_PERCENTAGE(setting) TO_STRING_SUFFIX_GETTER(setting,"%"), \
    UINT_RANGE_SETTER(setting,100)
#define UINT16(setting) TO_STRING_GETTER(setting), UINT_RANGE_SETTER(setting,UINT16_MAX)
#define UINT32(setting) TO_STRING_GETTER(setting), UINT_RANGE_SETTER(setting,UINT32_MAX)
#define DOUBLE_DURATION(setting) DOUBLE_DURATION_GETTER(setting), \
    DOUBLE_DURATION_SETTER(setting)

namespace MumblePluginBot
{
  std::list<SettingDescriptor> SettingDescriptors::create (Settings &settings)
  {
    std::list<SettingDescriptor> l;

    l.push_back ({"", "version", STRING(settings.version)});
    l.push_back ({"", "main_tempdir", PATH(settings.main_tempdir)});
    l.push_back ({"", "ducking", BOOL(settings.ducking)});
    l.push_back ({"", "ducking_vol", UINT_PERCENTAGE(settings.ducking_vol)});
    l.push_back ({"", "control_automute", BOOL(settings.control_automute)});
    l.push_back ({"", "chan_notify", UINT16(settings.chan_notify)});
    l.push_back ({"", "controlstring", STRING(settings.controlstring)});
    l.push_back ({"", "debug", BOOL(settings.debug)});
    l.push_back ({"", "verbose", BOOL(settings.verbose)});
    l.push_back ({"", "tick_duration", DOUBLE_DURATION(settings.tick_duration)});
    l.push_back ({"", "listen_to_private_message_only", BOOL(settings.listen_to_private_message_only)});
    l.push_back ({"", "listen_to_registered_users_only", BOOL(settings.listen_to_registered_users_only)});
    l.push_back ({"", "use_vbr", BOOL(settings.use_vbr)});
    l.push_back ({"", "stop_on_unregistered_users", BOOL(settings.stop_on_unregistered_users)});
    l.push_back ({"", "use_comment_for_status_display", BOOL(settings.use_comment_for_status_display)});
    // TODO: Blacklist entries: user_hash=username
    //std::map<std::string, std::string> blacklist;
    l.push_back ({"connection", "host", STRING(settings.connection.host)});
    l.push_back ({"connection", "port", UINT16(settings.connection.port)});
    l.push_back ({"connection", "username", STRING(settings.connection.username)});
    l.push_back ({"connection", "userpassword", STRING(settings.connection.userpassword)});
    l.push_back ({"connection", "targetchannel", STRING(settings.connection.targetchannel)});
    l.push_back ({"server_config", "allow_html", BOOL(settings.server_config.allow_html)});
    l.push_back ({"server_config", "message_length", UINT32(settings.server_config.message_length)});
    l.push_back ({"server_config", "image_message_length", UINT32(settings.server_config.image_message_length)});
    l.push_back ({"suggest_config", "version", STRING(settings.suggest_config.version)});
    l.push_back ({"suggest_config", "positional", BOOL(settings.suggest_config.positional)});
    l.push_back ({"suggest_config", "push_to_talk", BOOL(settings.suggest_config.push_to_talk)});

    // TODO: Add all the additional settings
    /*
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

    return l;
  }
}
