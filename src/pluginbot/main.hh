/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 netinetwalker
    Copyright (c) 2015 Natenom
    Copyright (c) 2016 Promi <prometheus@unterderbruecke.de>
    Copyright (c) 2017 Promi <prometheus@unterderbruecke.de>

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

#include "aither/log.hh"
#include "mumble/Mumble.pb.h"
#include "mumble/audio-player.hh"
#include "mumble/client.hh"
#include "pluginbot/settings.hh"
#include "pluginbot/plugin.hh"

namespace MumblePluginBot
{
  class Main;

  struct CommandArgs
  {
    const MumbleProto::TextMessage &msg;
    const std::string &command;
    const std::string &arguments;
    uint32_t msg_userid;
    std::list<std::unique_ptr<Plugin>> &plugins;
    Settings &settings;
    std::function<void(const std::string&)> reply;
    Mumble::Client &cli;
    Mumble::AudioPlayer &player;
    const Main &main;
  };

  struct Command
  {
    bool needs_binding;
    std::function<void(CommandArgs)> func;
  };

  class Main
  {
  public:
    Main (const Settings &settings,
          const std::string &config_filename, const Aither::Log &log);
    ~Main ();
    inline bool run () const
    {
      return m_run;
    }
    void disconnect ();
    int calc_overall_bandwidth (int framelength, int bitrate) const;
    int overall_bandwidth () const;
    void mumble_start ();
  private:
    bool m_run = false;
    bool m_duckthread_running = false;
    bool m_ticktimer_running = false;
    std::thread m_duckthread;
    std::thread m_ticktimer;
    Mumble::Configuration m_config;
    std::pair<Mumble::CertificatePaths, Mumble::Certificate> m_cert;
    std::unique_ptr<Mumble::Client> m_cli;
    std::unique_ptr<Mumble::AudioPlayer> m_player;
    Settings m_settings;
    Settings m_configured_settings;
    const Aither::Log &m_log;
    std::list<std::unique_ptr<Plugin>> m_plugins;
    std::string parse_cmd_options (int argc, char *argv[]);
    void timertick ();
    void handle_user_state_changes (const MumbleProto::UserState &msg);
    void handle_text_message (const MumbleProto::TextMessage &msg);
    void handle_text_message2 (const MumbleProto::TextMessage &msg,
                               const std::string &command, const std::string &arguments, uint32_t msg_userid);
    void start_duckthread ();
    void stop_duckthread ();
    void init_plugins ();

    static void about (CommandArgs &ca);
    static void settings (CommandArgs &ca);
    static void set (CommandArgs &ca);
    static void bind (CommandArgs &ca);
    static void blacklist (CommandArgs &ca);
    static void ducking (CommandArgs &ca);
    static void duckvol (CommandArgs &ca);
    static void bitrate (CommandArgs &ca);
    static void framesize (CommandArgs &ca);
    static void bandwidth (CommandArgs &ca);
    static void plugins (CommandArgs &ca);
    static void jobs (CommandArgs &ca);
    static void internals (CommandArgs &ca);
    static void help (CommandArgs &ca);
  };

}
