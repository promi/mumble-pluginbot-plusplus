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

#include "aither/log.hh"
#include "openssl/openssl.hh"
#include "pluginbot/main.hh"
#include "pluginbot/conf.hh"

std::string
parse_cmd_options (int argc, char *argv[], std::map <std::string, std::string> &settings)
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
          std::cout << "  -C --controllable <arg>  true if bot should be controllable from chat commands\n";
          std::cout << "  -d --certdir <directory> Path to cert\n";
          exit (0);
          break;
        case 2:
        case 'c':
          config_filename = optarg;
          break;
        case 3:
        case 'h':
          settings["mumbleserver_host"] = optarg;
          break;
        case 4:
        case 'p':
          settings["mumbleserver_port"] = optarg;
          break;
        case 5:
        case 'n':
          settings["mumbleserver_username"] = optarg;
          break;
        case 6:
        case 'u':
          settings["mumbleserver_userpassword"] = optarg;
          break;
        case 7:
        case 't':
          settings["mumbleserver_targetchannel"] = optarg;
          break;
        case 8:
        case 'b':
          settings["quality_bitrate"] = optarg;
          break;
        case 9:
        case 'f':
          settings["mpd_fifopath"] = optarg;
          break;
        case 10:
        case 'H':
          settings["mpd_host"] = optarg;
          break;
        case 11:
        case 'P':
          settings["mpd_port"] = optarg;
          break;
        case 12:
        case 'C':
          settings["controllable"] = optarg;
          break;
        case 13:
        case 'd':
          settings["certdirectory"] = optarg;
          break;
        }
    }
  return config_filename;
}

int
real_main (int argc, char *argv[])
{
  using namespace std::chrono_literals;
  setlocale (LC_ALL, "");
  OpenSSL::library_init ();
  std::map <std::string, std::string> settings;
  auto &&c = MumblePluginBot::std_config ();
  settings.insert (std::begin (c), std::end (c));
  const std::string config_filename {parse_cmd_options (argc, argv, settings)};
  Aither::Log m_log {settings["debug"] == "true" ? Aither::LogSeverity::Debug : Aither::LogSeverity::Verbose};
  AITHER_VERBOSE("Config loaded!");
  int i = 0;
  for (;;)
    {
      MumblePluginBot::Main client (settings, config_filename, m_log);
      AITHER_VERBOSE("pluginbot is starting...");
      client.init_settings ();
      AITHER_VERBOSE("start");
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
