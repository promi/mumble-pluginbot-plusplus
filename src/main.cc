/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 netinetwalker
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
#include <clocale>
#include <iostream>
#include <thread>
#include <chrono>
#include <getopt.h>
#include <experimental/filesystem>

#include "git-info.hh"
#include "aither/log.hh"
#include "openssl/openssl.hh"
#include "pluginbot/main.hh"
#include "pluginbot/settings.hh"

namespace fs = std::experimental::filesystem;

std::string
parse_cmd_options (int argc, char *argv[], MumblePluginBot::Settings &settings)
{
  std::string config_filename;
  static struct option long_options[] =
  {
    {"help", no_argument, nullptr, 1},
    {"config", required_argument, nullptr, 2},
    {"mumblehost", required_argument, nullptr, 3},
    {"mumbleport", required_argument, nullptr, 4},
    {"name", required_argument, nullptr, 5},
    {"userpass", required_argument, nullptr, 6},
    {"targetchannel", required_argument, nullptr, 7},
    {"bitrate", required_argument, nullptr, 8},
    {"fifo", required_argument, nullptr, 9},
    {"mpdhost", required_argument, nullptr, 10},
    {"mpdport", required_argument, nullptr, 11},
    {"controllable", required_argument, nullptr, 12},
    {"certdir", required_argument, nullptr, 13},
    {nullptr, 0, nullptr, 0}
  };
  int c;
  int option_index = 0;
  while ((c = getopt_long (argc, argv, "c:h:p:n:u:t:b:f:H:P:C:d:",
                           long_options, &option_index)) != -1)
    {
      switch (c)
        {
        case 1:
          std::cout << "Usage: " << argv[0] << " [options]\n";
          std::cout << "Options:\n";
          std::cout << "  -c --config <file>       Config filename\n";
          std::cout << "  -h --mumblehost <arg>    Mumble server IP or hostname\n";
          std::cout << "  -p --mumbleport <arg>    Mumble server port\n";
          std::cout << "  -n --name <arg>          Bot nickname\n";
          std::cout << "  -u --userpass <arg>      User password (optional)\n";
          std::cout << "  -t --targetchannel <arg> Auto join channel\n";
          std::cout << "  -b --bitrate <arg>       Desired audio bitrate\n";
          std::cout << "  -f --fifo <arg>          Path to FIFO file\n";
          std::cout << "  -H --mpdhost <arg>       MPD hostname\n";
          std::cout << "  -P --mpdport <arg>       MPD port\n";
          std::cout <<
                    "  -C --controllable <arg>  true if bot should be controllable from chat commands\n";
          std::cout << "  -d --certdir <directory> Path to cert\n";
          exit (0);
          break;
        case 2:
        case 'c':
          config_filename = optarg;
          break;
        case 3:
        case 'h':
          settings.connection.host = optarg;
          break;
        case 4:
        case 'p':
          settings.connection.port = std::stoi (optarg);
          break;
        case 5:
        case 'n':
          settings.connection.username = optarg;
          break;
        case 6:
        case 'u':
          settings.connection.userpassword = optarg;
          break;
        case 7:
        case 't':
          settings.connection.targetchannel = optarg;
          break;
        case 8:
        case 'b':
          settings.quality_bitrate = std::stoi (optarg);
          break;
        case 9:
        case 'f':
          settings.mpd.fifopath = optarg;
          break;
        case 10:
        case 'H':
          settings.mpd.host = optarg;
          break;
        case 11:
        case 'P':
          settings.mpd.port = std::stoi (optarg);
          break;
        case 12:
        case 'C':
          settings.controllable = optarg;
          break;
        case 13:
        case 'd':
          settings.certdir = optarg;
          break;
        }
    }
  return config_filename;
}

std::string
getenv_nullsafe (const std::string &name)
{
  const char* value = getenv (name.c_str ());
  if (value == nullptr)
    {
      return std::string {};
    }
  else
    {
      return std::string {value};
    }
}

void
set_dirs (MumblePluginBot::Settings &settings)
{
  // https://standards.freedesktop.org/basedir-spec/basedir-spec-0.8.html
  fs::path home = getenv_nullsafe ("HOME");
  assert (home != "");
  fs::path xdg_data_home = getenv_nullsafe ("XDG_DATA_HOME");
  if (xdg_data_home == "" || !xdg_data_home.is_absolute ())
    {
      xdg_data_home = home / ".local" / "share";
    }
  assert (xdg_data_home != "");
  fs::path xdg_runtime_dir = getenv_nullsafe ("XDG_RUNTIME_DIR");
  if (xdg_runtime_dir == "" || !xdg_runtime_dir.is_absolute ())
    {
      std::cerr << "warning: XDG_RUNTIME_DIR is not set" << std::endl;
      xdg_runtime_dir = home / ".config";
    }
  assert (xdg_runtime_dir != "");
  fs::path xdg_config_home = getenv_nullsafe ("XDG_CONFIG_HOME");
  if (xdg_config_home == "" || !xdg_config_home.is_absolute ())
    {
      xdg_config_home = home / ".config";
    }
  assert (xdg_config_home != "");

  const std::string &subdir = "mumble-pluginbot-plusplus";
  settings.main_tempdir = xdg_data_home / subdir / "temp";
  settings.mpd.fifopath = xdg_runtime_dir / subdir / "mpd.fifo";
  settings.mpd.musicdir = home / "mpd" / "music";
  settings.certdir = xdg_config_home / subdir / "cert";
}

int
real_main (int argc, char *argv[])
{
  using namespace std::chrono_literals;
  // Use the users locale, but ...
  setlocale (LC_ALL, "");
  // Don't use the numeric part.
  // The decimal separator should always be a dot '.'!
  // That fact is used for string to setting and setting to string conversion
  setlocale (LC_NUMERIC, "C");
  OpenSSL::library_init ();
  MumblePluginBot::Settings settings;
  set_dirs (settings);
  const std::string config_filename
  {
    parse_cmd_options (argc, argv, settings)
  };
  // TODO: Implement config file parsing and loading here
  auto severity = Aither::LogSeverity::Warning;
  if (settings.verbose)
    {
      severity = Aither::LogSeverity::Verbose;
    }
  if (settings.debug)
    {
      severity = Aither::LogSeverity::Debug;
    }
  Aither::Log m_log {severity};
  AITHER_VERBOSE("Config loaded!");
  int i = 0;
  for (;;)
    {
      MumblePluginBot::Main client (settings, config_filename, m_log);
      AITHER_VERBOSE("pluginbot (" << GIT_DESCRIBE << ") is starting...");
      client.mumble_start ();
      try
        {
          while (client.run ())
            {
              std::this_thread::sleep_for (0.5s);
              i++;
              if (i > 60)
                {
                  //return 1;
                }
            }
        }
      catch (...)
        {
          AITHER_ERROR("An error occurred: #{$!}");
          AITHER_ERROR("Backtrace: #{$@}");
          client.disconnect ();
        }
      AITHER_VERBOSE("");
      AITHER_VERBOSE("----------------------------------------------");
      AITHER_VERBOSE("-- Restart                                  --");
      AITHER_VERBOSE("----------------------------------------------");
      AITHER_DEBUG("Sleeping for 3 seconds");
      std::this_thread::sleep_for (3s);
    }
  return 0;
}

int
main (int argc, char *argv[])
{
  try
    {
      return real_main (argc, argv);
    }
  catch (const std::string &s)
    {
      std::cerr << "An error occured: " << s << std::endl;
    }
  catch (const std::exception &e)
    {
      std::cerr << "An error occured: " << e.what () << std::endl;
    }
}
