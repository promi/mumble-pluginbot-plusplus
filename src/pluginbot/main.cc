/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 netinetwalker
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
#include <iostream>
#include <memory>
#include <thread>
#include <functional>
#include <algorithm>

#include "git-info.hh"
#include "mumble/configuration.hh"
#include "pluginbot/main.hh"
#include "pluginbot/plugin.hh"
#include "pluginbot/settings.hh"
#include "pluginbot/html.hh"
#include "pluginbot/setting-descriptors.hh"

#include "pluginbot/plugins/mpd.hh"
#include "pluginbot/plugins/version.hh"
#include "pluginbot/plugins/youtube.hh"

namespace MumblePluginBot
{
  const std::string org_source_url =
    "https://github.com/promi/mumble-pluginbot-plusplus/";
  const std::string org_doc_url =
    "https://github.com/promi/mumble-pluginbot-plusplus/";
  const std::string org_license_url =
    "https://github.com/promi/mumble-pluginbot-plusplus/blob/master/COPYING/";
  const std::string org_issues_url =
    "https://github.com/promi/mumble-pluginbot-plusplus/issues/";
  const std::string org_wiki_url =
    "https://github.com/promi/mumble-pluginbot-plusplus/";

  Main::Main (const Settings &settings,
              const std::string &config_filename,
              const Aither::Log &log) : m_settings (settings), m_log (log)
  {
    m_run = false;
    m_config.host = m_settings.connection.host;
    m_config.port = m_settings.connection.port;
    m_config.username = m_settings.connection.username;
    m_config.password = m_settings.connection.userpassword;
    m_config.bitrate = m_settings.quality_bitrate;
    m_config.vbr_rate = m_settings.use_vbr;
    m_config.ssl_cert_opts.cert_dir = m_settings.certdir;
    m_cli = std::make_unique<Mumble::Client> (m_log, m_config, std::string ("mumble-pluginbot-plusplus ") + GIT_DESCRIBE);
    if (config_filename != "")
      {
        AITHER_VERBOSE("parse extra config");
        /*
          if File.exist? v
          begin
          require_relative v
          ext_config()
          rescue
          std::cout << "Your config could not be loaded!\n";
          end
          else
          std::cout << "Config path- and/or filename is wrong!\n";
          std::cout << "used //{v}\n";
          std::cout << "Config not loaded!\n";
          end
        */
      }
    m_configured_settings = m_settings;
  }

  Main::~Main ()
  {
    stop_duckthread ();
    try
      {
        m_ticktimer_running = false;
        m_ticktimer.join ();
      }
    catch (const std::system_error &e)
      {
        AITHER_WARNING("Can't join timertick thread");
      }
  }

  void Main::disconnect ()
  {
    if (m_cli->connected ())
      {
        m_cli->disconnect ();
      }
  }

  int Main::calc_overall_bandwidth (int framelength, int bitrate) const
  {
    return (1000.f / framelength * 320.f) + bitrate;
  }

  int Main::overall_bandwidth () const
  {
    return calc_overall_bandwidth(m_cli->frame_length ().count (),
                                  m_cli->bitrate ());
  }

  void Main::start_duckthread ()
  {
    using namespace std::chrono_literals;
    auto &player = m_cli->player ();
    if (m_duckthread_running)
      {
        return;
      }
    m_duckthread_running = true;
    m_duckthread = std::thread {[&] {
        while (m_duckthread_running && player.volume () < 100)
          {
            player.volume (player.volume () + 2);
            std::this_thread::sleep_for (20ms);
          }
      }
    };
  }

  void Main::stop_duckthread ()
  {
    try
      {
        m_duckthread_running = false;
        m_duckthread.join ();
      }
    catch (...)
      {
        AITHER_DEBUG("[killduckthread] can't kill because #{$!}");
      }
  }

  void Main::mumble_start ()
  {
    using namespace std::chrono_literals;
    m_cli->on<MumbleProto::ServerConfig> ([&] (auto serverconfig)
    {
      m_settings.server_config.image_message_length =
        serverconfig.image_message_length ();
      m_settings.server_config.message_length = serverconfig.message_length ();
      m_settings.server_config.allow_html = serverconfig.allow_html ();
    });

    m_cli->on<MumbleProto::SuggestConfig> ([&] (auto suggestconfig)
    {
      m_settings.suggest_config.version = suggestconfig.version ();
      m_settings.suggest_config.positional = suggestconfig.positional ();
      m_settings.suggest_config.push_to_talk = suggestconfig.push_to_talk ();
    });

    m_cli->connect ();
    //for (;;)
    //  {
    //    std::this_thread::sleep_for (0.5s);
    //  }
    // TODO: There should be some setting for this
    for (int i = 0; i < 10; i++)
      {
        if (m_cli->connected ())
          {
            break;
          }
        AITHER_DEBUG("Connecting to the server is still ongoing.");
        std::this_thread::sleep_for (0.5s);
      }
    if (!m_cli->connected ())
      {
        m_cli->disconnect ();
        AITHER_DEBUG("Connection timed out");
        return;
      }
    AITHER_VERBOSE("connected");
    // Allow some time for server sync
    // TODO: There should be some setting for this
    std::this_thread::sleep_for (0.5s);
    for (int i = 0; i < 10; i++)
      {
        if (m_cli->synced ())
          {
            break;
          }
        AITHER_DEBUG("Server sync is still ongoing.");
        std::this_thread::sleep_for (0.5s);
      }
    if (!m_cli->synced ())
      {
        AITHER_DEBUG("Server sync timed out");
        m_cli->disconnect ();
        return;
      }
    {
      const std::string &targetchannel = m_settings.connection.targetchannel;
      if (targetchannel != "")
        {
          try
            {
              m_cli->join_channel (targetchannel);
            }
          catch (...)
            {
              AITHER_DEBUG("[joincannel] Can't join " + targetchannel + "!");
            }
        }
    }
    m_cli->on<MumbleProto::UserState> ([&] (const auto &msg)
    {
      this->handle_user_state_changes (msg);
    });
    m_cli->on<MumbleProto::TextMessage> ([&] (const auto &msg)
    {
      this->handle_text_message (msg);
    });
    m_cli->on<MumbleProto::UDPTunnel> ([&] (const auto &_)
    {
      (void)_;
      if (m_settings.ducking)
        {
          m_cli->player ().volume (m_settings.ducking_vol);
          this->start_duckthread ();
        }
    });
    m_run = true;
    // m_cli->player ().stream_named_pipe (m_settings.mpd.fifopath);
    init_plugins ();
    m_ticktimer_running = true;
    m_ticktimer = std::thread
    {
      [&] (void)
      {
        timertick ();
      }
    };
  }

  void Main::init_plugins ()
  {
    // Load all plugins
    // TODO: Load from dynamic plugin libraries in a system-wide and a user plugin dir
    // Dir["./plugins/ *.rb"].each do |f|
    auto &player = m_cli->player ();
    m_plugins.push_back (std::make_unique<VersionPlugin> (m_log, m_settings, *m_cli,
                         player));
    auto messages_ptr = std::make_unique<MessagesPlugin> (m_log, m_settings, *m_cli,
                        player);
    MessagesPlugin &messages = *messages_ptr;
    m_plugins.push_back (std::move (messages_ptr));
    m_plugins.push_back (std::make_unique<MpdPlugin> (m_log, m_settings, *m_cli,
                         player, messages));
    m_plugins.push_back (std::make_unique<YoutubePlugin> (m_log, m_settings, *m_cli,
                         player));
    for (auto &plugin : m_plugins)
      {
        AITHER_VERBOSE("Plugin '" << plugin->name () << "' loaded.");
      }
    for (auto &plugin : m_plugins)
      {
        plugin->init ();
      }
    // Sort by name, so all plugin related user command output is in alphabetical order
    m_plugins.sort ([] (auto &a, auto &b)
    {
      return a->name () < b->name ();
    });
  }

  void Main::timertick ()
  {
    using namespace std::chrono_literals;
    auto tick_duration = m_settings.tick_duration;
    while (m_ticktimer_running)
      {
        std::this_thread::sleep_for (tick_duration);
        auto time = std::chrono::system_clock::now ();
        // AITHER_DEBUG ("tick");
        for (auto& plugin : m_plugins)
          {
            plugin->tick (time);
          }
      }
  }

  void Main::handle_user_state_changes (const MumbleProto::UserState &msg)
  {
    (void) msg;
    // msg.actor = session_id of user who did something on someone,
    //             if self done, both is the same.
    // msg.session = session_id of the target
  }

  void Main::handle_text_message (const MumbleProto::TextMessage &msg)
  {
    AITHER_DEBUG("text message: " << msg.message ());
    if (!msg.has_actor ())
      {
        AITHER_DEBUG("no actor");
        // ignore text messages from the server
        return;
      }
    auto actor = msg.actor ();
    // This is hacky because mumble uses -1 for user_id of unregistered users,
    // while mumble-ruby seems to just omit the value for unregistered users.
    // With this hacky thing commands from SuperUser are also being ignored.
    uint32_t msg_userid = (uint32_t) -1;
    bool sender_is_registered = false;
    auto &user = m_cli->users ().at (actor);
    if (user.has_user_id ())
      {
        msg_userid = user.user_id ();
        sender_is_registered = true;
      }
    // user on a blacklist?
    if (m_settings.blacklist.find (user.hash ()) != std::end (m_settings.blacklist))
      {
        // virtually unregister
        sender_is_registered = false;
        AITHER_DEBUG("user in blacklist!");
      }
    if (m_settings.superpassword != "")
      {
        if (msg.message () == m_settings.superpassword + "restart")
          {
            m_settings = m_configured_settings;
            m_cli->text_channel (m_cli->me ().channel_id (), m_settings.superanswer);
            m_run = false;
            m_cli->disconnect ();
          }

        if (msg.message () == m_settings.superpassword + "reset")
          {
            m_settings = m_configured_settings;
            m_cli->text_channel (m_cli->me ().channel_id (), m_settings.superanswer);
          }
      }

    if (!sender_is_registered && m_settings.listen_to_registered_users_only)
      {
        AITHER_DEBUG("Debug: Not listening because "
                     "'listen_to_registered_users_only' is 'true' "
                     "and sender is unregistered or on a blacklist.");
        return;
      }

    // Check whether message is a private one or was sent to the channel.
    // Channel messages don't have a session, so skip them
    if (!msg.session_size () && m_settings.listen_to_private_message_only)
      {
        AITHER_DEBUG("Debug: Not listening because "
                     "'listen_to_private_message_only' is 'true' "
                     "and message was sent to channel.");
        return;
      }
    if (!m_settings.controllable)
      {
        AITHER_DEBUG("Not listening, because controllable is 'false'");
        return;
      }
    // message consists of: control_string + command [+ space + arguments]
    std::string message = msg.message ();
    const std::string &cs = m_settings.controlstring;
    auto cs_size = cs.size ();
    // Check whether we have a command after the controlstring.
    if (message.size () <= cs_size || message.compare (0, cs_size, cs))
      {
        AITHER_DEBUG("no control string in text message");
        return;
      }
    // remove control string
    message = message.substr (cs_size);
    auto space_pos = message.find (' ');
    const std::string command = message.substr (0, space_pos);
    std::string arguments;
    if (space_pos != std::string::npos && space_pos + 1 < message.size ())
      {
        arguments = message.substr (space_pos + 1);
      }
    auto reply = [&] (auto msg)
    {
      m_cli->text_user (actor, msg);
    };
    try
      {
        handle_text_message2 (msg, command, arguments, msg_userid);
      }
    catch (std::invalid_argument &e)
      {
        reply (std::string ("Invalid argument: ") + e.what ());
      }
    catch (std::out_of_range &e)
      {
        reply (std::string ("Value was out of range: ") + e.what ());
      }
    catch (std::exception &e)
      {
        reply (std::string {"Unknown error: "} + e.what ());
      }
    catch (...)
      {
        reply ("Unknown error");
      }
  }

  void Main::handle_text_message2 (const MumbleProto::TextMessage &msg,
                                   const std::string &command,
                                   const std::string &arguments,
                                   uint32_t msg_userid)
  {
    AITHER_DEBUG("Text message was parsed, searching for function to call");
    const auto &actor = msg.actor ();
    for (auto &plugin : m_plugins)
      {
        plugin->chat (msg, command, arguments);
      }

    //std::function<void(const std::string&)> reply = [this, actor] (auto msg)
    auto reply = [this, actor] (auto msg)
    {
      m_cli->text_user (actor, msg);
    };

    std::map<std::string, Command> commands =
    {
      {
        "about", {
          false, [] (auto ca)
          {
            about (ca);
          }
        }
      },
      {
        "settings", {
          false, [] (auto ca)
          {
            settings (ca);
          }
        }
      },
      {
        "set", {
          true, [] (auto ca)
          {
            set (ca);
          }
        }
      },
      {
        "bind", {
          false, [] (auto ca)
          {
            bind (ca);
          }
        }
      },
      {
        "unbind", {
          true, [] (auto ca)
          {
            ca.settings.boundto = "";
          }
        }
      },
      {
        "reset", {
          true, [this] (auto ca)
          {
            (void) ca;
            m_settings = m_configured_settings;
          }
        }
      },
      {
        "restart", {
          true, [this] (auto ca)
          {
            m_run = false;
            ca.cli.disconnect ();
          }
        }
      },
      {
        "register", {
          true, [this] (auto ca)
          {
            ca.cli.register_self ();
          }
        }
      },
      {
        "blacklist", {
          true, [] (auto ca)
          {
            blacklist (ca);
          }
        }
      },
      {
        "ducking", {
          false, [] (auto ca)
          {
            ducking (ca);
          }
        }
      },
      {
        "duckvol", {
          false, [] (auto ca)
          {
            duckvol (ca);
          }
        }
      },
      {
        "bitrate", {
          false, [] (auto ca)
          {
            bitrate (ca);
          }
        }
      },
      {
        "framesize", {
          false, [] (auto ca)
          {
            framesize (ca);
          }
        }
      },
      {
        "bandwidth", {
          false, [] (auto ca)
          {
            bandwidth (ca);
          }
        }
      },
      {
        "plugins", {
          false, [] (auto ca)
          {
            plugins (ca);
          }
        }
      },
      {
        "jobs", {
          false, [] (auto ca)
          {
            jobs (ca);
          }
        }
      },
      {
        "internals", {
          false, [] (auto ca)
          {
            internals (ca);
          }
        }
      },
      {
        "help", {
          false, [] (auto ca)
          {
            help (ca);
          }
        }
      }
    };

    CommandArgs ca = {msg, command, arguments, msg_userid, m_plugins, m_settings, reply,
                      *m_cli, *this
                     };
    bool boundto_msg_user = m_settings.boundto == std::to_string (msg_userid);

    const auto it = commands.find (command);
    if (it != std::end (commands))
      {
        if (!it->second.needs_binding || boundto_msg_user)
          {
            it->second.func (ca);
          }
        else
          {
            reply ("This command can only be issued by the bound user!");
          }
      }
  }

  void Main::about (CommandArgs &ca)
  {
    std::stringstream header;
    header << br_tag + "Hi, I am a mumble-pluginbot-plusplus bot." << br_tag;
    std::stringstream out;
    out << li_tag (a_tag (org_source_url, "Get my source code"));
    out << li_tag (a_tag (org_doc_url, "Read my documentation"));
    out << li_tag ("I am licensed under the " + a_tag (org_license_url,
                   "AGPLv3 license"));
    out << li_tag ("If you have any issues, bugs or ideas please tell us on " +
                   a_tag (org_issues_url, org_issues_url));
    ca.reply (header.str () + ul_tag (out.str ()));
  }

  void Main::settings (CommandArgs &ca)
  {
    auto descriptors = SettingDescriptors::create (ca.settings);
    descriptors.sort ([] (auto &a, auto &b)
    {
      if (a.section == b.section)
        {
          return a.name < b.name;
        }
      else
        {
          return a.section < b.section;
        }
    });
    std::stringstream out;
    for (const auto &it : descriptors)
      {
        if (it.name != "logo" && it.name != "superanswer")
          {
            out << tr_tag (td_tag (it.section + '.' + it.name) +
                           td_tag (it.to_string ())) << "\n";
          }
      }
    ca.reply (table_tag (out.str ()));
  }

  void Main::set (CommandArgs &ca)
  {
    const std::string &arguments = ca.arguments;
    auto equals_pos = arguments.find ('=');
    if (equals_pos == std::string::npos)
      {
        ca.reply ("Invalid arguments for this command!");
        return;
      }
    const std::string key = arguments.substr (0, equals_pos);
    const std::string val = arguments.substr (equals_pos + 1);
    auto descriptors = SettingDescriptors::create (ca.settings);
    auto it = std::find_if (std::begin (descriptors), std::end (descriptors),
                            [&] (auto descriptor)
    {
      return descriptor.name == key;
    });
    if (it == std::end (descriptors))
      {
        ca.reply ("Setting '" + key + "' not found!");
        return;
      }
    it->from_string (val);
    ca.reply ("Setting '" + key + "' updated.");
  }

  void Main::bind (CommandArgs &ca)
  {
    if (ca.settings.boundto == "")
      {
        ca.settings.boundto = std::to_string (ca.msg_userid);
      }
  }

  void Main::blacklist (CommandArgs &ca)
  {
    const std::string &username = ca.arguments;
    Mumble::User* user = ca.cli.find_user (username);
    if (user)
      {
        const std::string hash_text = user->hash ();
        ca.settings.blacklist[hash_text] = username;
        ca.reply ("This ban is active until the bot restarts. "
                  "To permaban add following line to your configuration:");
        ca.reply (hash_text + "=" + username);
      }
    else
      {
        ca.reply ("User not found: " + username);
      }
  }

  std::string bonoff (bool b)
  {
    return b ? "on" : "off";
  }

  std::string btruefalse (bool b)
  {
    return b ? "true" : "false";
  }

  void Main::ducking (CommandArgs &ca)
  {
    bool ducking = ca.settings.ducking = !ca.settings.ducking;
    ca.reply (std::string {"Music ducking is "} + bonoff (ducking)  + ".");
  }

  void Main::duckvol (CommandArgs &ca)
  {
    if (ca.arguments == "")
      {
        std::stringstream ss;
        ss << "Ducking volume is set to " << ca.settings.ducking_vol <<
           "% of normal volume. Ducking itself it set to: " << btruefalse (
             ca.settings.ducking) << ".";
        ca.reply (ss.str ());
      }
    else
      {
        int volume = std::stoi (ca.arguments);
        if (volume >= 0 && volume <= 100)
          {
            ca.settings.ducking_vol = volume;
            ca.reply ("Ducking is set to " + std::to_string (volume) +
                      "% of normal volume.");
          }
        else
          {
            ca.reply ("Volume must be within a range of 0 to 100.");
          }
      }
  }

  void Main::bitrate (CommandArgs &ca)
  {
    if (ca.arguments == "")
      {
        const std::string &bitrate = std::to_string (ca.cli.bitrate ());
        ca.reply ("Encoding is set to " + bitrate + " bit/s.");
      }
    else
      {
        int bitrate = std::stoi (ca.arguments);
        ca.cli.bitrate (bitrate);
        ca.reply ("Encoding is set now to " + std::to_string (bitrate) + " bit/s.");
        ca.reply ("The calculated overall bandwidth is " + std::to_string (
                    ca.main.overall_bandwidth ()) + " bit/s.");
      }
  }

  void Main::framesize (CommandArgs &ca)
  {
    if (ca.arguments == "")
      {
        std::chrono::milliseconds frame_length = ca.cli.frame_length ();
        ca.reply ("Sending in " + std::to_string (frame_length.count ()) +
                  " ms frames.");
      }
    else
      {
        std::chrono::milliseconds frame_length {std::stoi (ca.arguments)};
        ca.cli.frame_length (frame_length);
        ca.reply ("Sending now in " + std::to_string (frame_length.count ()) +
                  " ms frames.");
        ca.reply ("The calculated overall bandwidth is " + std::to_string (
                    ca.main.overall_bandwidth ()) + " bit/s.");
        ca.reply ("Server settings " + std::to_string (ca.cli.max_bandwidth ()) +
                  " bit/s.");
      }
  }

  void Main::bandwidth (CommandArgs &ca)
  {
    ca.reply (br_tag + u_tag ("Current bandwidth related settings:") + br_tag +
              "The calculated overall bandwidth (audio + overhead): " + std::to_string (
                ca.main.overall_bandwidth ()) + " bit/s" + br_tag +
              " Audio encoding bandwidth: " + std::to_string (ca.cli.bitrate ()) + " bit/s" +
              br_tag +
              " Framesize: " + std::to_string (ca.cli.frame_length ().count ()) + " ms");
  }

  void Main::plugins (CommandArgs &ca)
  {
    std::stringstream ss;
    for (auto& plugin : ca.plugins)
      {
        ss << plugin->name () << br_tag;
      }
    std::string help = br_tag + red_span ("Loaded plugins:" + br_tag + b_tag (
                                            ss.str ()));
    auto cs = ca.settings.controlstring;
    help += br_tag + b_tag (cs + "help " + i_tag ("pluginname")) +
            " Get the help text for the specific plugin." + br_tag + br_tag +
            "For example send the following text to " +
            "get some basic control commands of the bot:" + br_tag + b_tag (
              cs + "help mpd") + br_tag;
    ca.reply (help);
  }

  void Main::jobs (CommandArgs &ca)
  {
    (void) ca;
    // TODO: Removed, but there is experimental code in the original MRPB
  }

  void Main::internals (CommandArgs &ca)
  {
    const auto &cs = ca.settings.controlstring;

    std::string help {br_tag + red_span (b_tag ("Internal commands")) + br_tag};
    help += b_tag ("superpassword+restart") + " will restart the bot." + br_tag;
    help += b_tag ("superpassword+reset") + " will reset variables to start values."
            + br_tag;
    help += b_tag (cs + "about") + " Get information about this bot." + br_tag;
    help += b_tag (cs + "settings") + " display current settings." + br_tag;
    help += b_tag (cs + "set " + i_tag ("variable=value")) +
            " Set variable to value." + br_tag;
    help += b_tag (cs + "bind") +
            " Bind bot to a user. " + "(some functions will only work if bot is bound)." +
            br_tag;
    help += b_tag (cs + "unbind") + " Unbind bot." + br_tag;
    help += b_tag (cs + "reset") +
            " Reset variables to default value. Needs binding!" + br_tag;
    help += b_tag (cs + "restart") + " Restart Bot. Needs binding." + br_tag;
    help += b_tag (cs + "blacklist " + i_tag ("username")) +
            " Add user to blacklist. Needs binding." + br_tag;
    help += b_tag (cs + "register") +
            " Let the bot register itself on the current server. " +
            "Works only if server allows it. " +
            "If it doesn't work ask an administrator of your Mumble server. " +
            "Be aware that after registration only " +
            "an administrator can change the name of the bot."
            + br_tag;
    help += b_tag (cs + "ducking") + " Toggle voice ducking on/off." + br_tag;
    help += b_tag (cs + "duckvol " + i_tag ("volume")) +
            " Set the ducking volume (% of normal volume)." + br_tag;
    help += b_tag (cs + "duckvol") + " Show current ducking volume." + br_tag;
    help += b_tag (cs + "bitrate " + i_tag ("rate in kbit/s")) +
            " Set audio encoding rate. " +
            "Note that the bot needs additional bandwidth for overhead "+
            "so the overall bandwidth is higher than this bitrate."
            + br_tag;
    help += b_tag (cs + "bandwidth") +
            " Show information about the overall bandwidth, " +
            "audio bandwidth (bitrate) and framesize."
            + br_tag;

    ca.reply (help);
  }

  void Main::help (CommandArgs &ca)
  {
    if (ca.arguments == "all")
      {
        // Send help texts of all plugins.
        std::string help;
        for (auto& plugin : ca.plugins)
          {
            ca.reply (plugin->help ());
          }
      }
    else if (ca.arguments != "")
      {
        // Send help for a specific plugin.
        auto it = std::find_if (std::begin (ca.plugins),
                                std::end (ca.plugins), [&] (auto& plugin)
        {
          return plugin->name () == ca.arguments;
        });
        if (it != std::end (ca.plugins))
          {
            ca.reply ((*it)->help ());
          }
      }
    else
      {
        // Send default help text.
        const auto &cs = ca.settings.controlstring;
        std::string help = br_tag;
        help += "Hi, I am a " + a_tag (org_wiki_url, "mumble-pluginbot-plusplus") +
                " and YOU can control me through text commands." + br_tag;
        help += br_tag;
        help += "I will give you a good start with the basic commands you "
                "need to control the music I have to offer :) - if you "
                "send me the following command:" + br_tag;
        help += b_tag (cs + "help mpd") + br_tag;
        help += br_tag;
        help += "If you are more interested in who/what I am, send to me:" + br_tag;
        help += b_tag (cs + "about") + br_tag;
        help += br_tag;
        help += b_tag (u_tag ("Commands for advanced users:")) + br_tag;
        help += b_tag (cs + "plugins") + " - Get a list of available plugins." + br_tag;
        help += br_tag;
        help += "Note: Every plugin has its own help text; to get it send the command: "
                + br_tag;
        help += b_tag (cs + "help name_of_the_plugin") + br_tag;
        help += "For example: " + br_tag;
        help += b_tag (cs + "help mpd") + br_tag;
        help += br_tag;
        help += b_tag (u_tag ("Commands for experts only:")) + br_tag;
        help += b_tag (cs + "internals") + " - See my internal commands." + br_tag;
        ca.reply (help);
      }
  }
}
