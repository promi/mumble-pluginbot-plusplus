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
#include <iostream>
#include <memory>
#include <thread>
#include <functional>

#include "mumble/configuration.hh"
#include "pluginbot/main.hh"
#include "pluginbot/plugin.hh"
#include "pluginbot/conf.hh"

namespace MumblePluginBot
{
  const std::string org_source_url =
    "https://github.com/dafoxia/mumble-ruby-pluginbot/";
  const std::string org_doc_url =
    "https://wiki.natenom.com/w/Mumble-Ruby-Pluginbot/";
  const std::string org_license_url =
    "https://github.com/dafoxia/mumble-ruby-pluginbot/blob/master/LICENSE/";
  const std::string org_issues_url =
    "https://github.com/dafoxia/mumble-ruby-pluginbot/issues/";
  const std::string org_wiki_url =
    "https://wiki.natenom.com/w/Mumble-Ruby-Pluginbot/";

  Main::Main (const std::map <std::string, std::string> &settings,
              const std::string &config_filename,
              const Aither::Log &log) : m_settings (settings), m_log (log)
  {
    // load all plugins
    /*
      Dir["./plugins/ *.rb"].each do |f|
      require f
      std::cout << "Plugin //{f} loaded."
      end
    */
    if (config_filename != "")
      {
        AITHER_VERBOSE("parse extra config");
        /*
          if File.exist? v
          begin
          require_relative v
          ext_config()
          rescue
          std::cout << "Your config could not be loaded!" << std::endl;
          end
          else
          std::cout << "Config path- and/or filename is wrong!" << std::endl;
          std::cout << "used //{v}" << std::endl;
          std::cout << "Config not loaded!" << std::endl;
          end
        */
      }
    if (m_settings.find ("ducking_volume") == std::end (m_settings))
      {
        m_settings["ducking_volume"] = 20;
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


  void Main::init_settings ()
  {
    m_run = false;
    m_cli = std::make_unique<Mumble::Client> (m_settings["mumbleserver_host"],
                                              std::stoi (m_settings["mumbleserver_port"]),
                                              m_settings["mumbleserver_username"],
                                              m_settings["mumbleserver_userpassword"], [&] (auto conf)
                                              {
                                                conf.bitrate = std::stoi (m_settings["quality_bitrate"]);
                                                conf.vbr_rate = std::stoi (m_settings["use_vbr"]);
                                                conf.ssl_cert_opts.cert_dir = m_settings["certdirectory"];
                                              });
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
    m_settings["mumbleserver_imagelength"] = serverconfig.image_message_length ();
    m_settings["mumbleserver_messagelength"] = serverconfig.message_length ();
    m_settings["mumbleserver_allow_html"] = serverconfig.allow_html ();
  });

  m_cli->on<MumbleProto::SuggestConfig> ([&] (auto suggestconfig)
  {
    m_settings["mumbleserver_version"] = suggestconfig.version ();
    m_settings["mumbleserver_positional"] = suggestconfig.positional ();
    m_settings["mumbleserver_push_to_talk"] = suggestconfig.push_to_talk ();
  });

  m_cli->connect ();
  //for (;;)
  //  {
  //    std::this_thread::sleep_for (0.5s);
  //  }
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
  std::this_thread::sleep_for (0.1s);
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
    const std::string &targetchannel = m_settings["mumbleserver_targetchannel"];
    try
      {
        m_cli->join_channel (targetchannel);
      }
    catch (...)
      {
        AITHER_DEBUG("[joincannel] Can't join " + targetchannel + "!");
      }
  }
  m_cli->comment ("");
  m_settings["set_comment_available"] = "true";
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
    if (m_settings["ducking"] == "true")
      {
        m_cli->player ().volume ((std::stoi (m_settings["ducking_volume"]) |  0x1) - 1);
        this->start_duckthread ();
      }
  });
  m_run = true;
  m_cli->player ().stream_named_pipe (m_settings["mpd_fifopath"]);
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
  /*
    #init all plugins
    init = @settings.clone
    init[:cli] = @cli

    std::cout << "initplugins" << std::endl;
    Plugin.plugins.each do |plugin_class|
    @plugin << plugin_class.new
    end

    maxcount = @plugin.length
    allok = 0
    while allok != @plugin.length do
    allok = 0
    @plugin.each do |plugin|
    init = plugin.init(init)
    if plugin.name != "false"
    allok += 1
    end
    end
    maxcount -= 1
    break if maxcount <= 0
    end
    std::cout << "maybe not all plugins functional!" << std::endl; if maxcount <= 0
  */
}

  void Main::timertick ()
  {
    using namespace std::chrono_literals;
    int ticks_per_hour = 3600;
    try
      {
        ticks_per_hour = std::stoi (m_settings["ticks_per_hour"]);
      }
    catch(std::invalid_argument)
      {
      }
    while (m_ticktimer_running)
      {
        // TODO: The commented out part is WRONG.
        // It sleeps for a very short amount of time and makes the process use 100% CPU
        // So find out how to do proper math with std::chrono here!
        // OTOH: Keeping it at 1s should be fine for now.
        (void) ticks_per_hour;
        std::this_thread::sleep_for (1s); // (1h / ticks_per_hour);
        auto time = std::chrono::system_clock::now ();
        (void) time;
        /*
          @plugin.each do |plugin|
          plugin.ticks(time)
          end
        */
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
    if (!msg.has_actor ())
      {
        // ignore text messages from the server
        return;
      }
    // This is hacky because mumble uses -1 for user_id of unregistered users,
    // while mumble-ruby seems to just omit the value for unregistered users.
    // With this hacky thing commands from SuperUser are also being ignored.
    uint32_t msg_userid = (uint32_t) -1;
    bool sender_is_registered = false;
    auto &user = m_cli->users ().at (msg.actor ());
    if (user.has_user_id ())
      {
        msg_userid = user.user_id ();
        sender_is_registered = true;
      }
    // user on a blacklist?
    if (m_settings.find (user.hash ()) != std::end (m_settings))
      {
        // virtually unregister
        sender_is_registered = false;
        if (m_settings["debug"] == "true")
          {
            std::cout << "user in blacklist!" << std::endl;
          }
      }
    // FIXME: strip html tags.
    // BEFORE doing this we need to ensure that no plugin needs the html
    // source code. For example youtube plugin needs them...
    //msg.message.gsub!(/(<[^<^>]*>)/, "")

    if (msg.message () == m_settings["superpassword"] + "restart")
      {
        m_settings = m_configured_settings;
        m_cli->text_channel (m_cli->me ().channel_id (),
                             m_settings["superanswer"]);
        m_run = false;
        m_cli->disconnect ();
      }

    if (msg.message () == m_settings["superpassword"] + "reset")
      {
        m_settings = m_configured_settings;
        m_cli->text_channel (m_cli->me ().channel_id (),
                             m_settings["superanswer"]);
      }

    if (!sender_is_registered
        && m_settings["listen_to_registered_users_only"] == "true")
      {
        if (m_settings["debug"] == "true")
          {
            std::cout <<
              "Debug: Not listening because "
              "'listen_to_registered_users_only' is 'true' "
              "and sender is unregistered or on a blacklist."
                      << std::endl;
          }
        return;
      }

    // Check whether message is a private one or was sent to the channel.
    // Channel messages don't have a session, so skip them
    if (!msg.session_size ()
        && m_settings["listen_to_private_message_only"] == "true")
      {
        if (m_settings["debug"] == "true")
          {
            std::cout <<
              "Debug: Not listening because "
              "'listen_to_private_message_only' is 'true' "
              "and message was sent to channel."
                      << std::endl;
          }
        return;
      }
    if (m_settings["controllable"] != "true")
      {
        return;
      }
    // message consists of: control_string + command [+ space + arguments]
    std::string message = msg.message ();
    const std::string &cs = m_settings["controlstring"];
    auto cs_size = cs.size ();
    // Check whether we have a command after the controlstring.
    if (message.size () <= cs_size || message.compare (0, cs_size, cs))
      {
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
    handle_text_message2 (msg, command, arguments, msg_userid);
  }

  std::string a_tag (const std::string &url, const std::string &label)
  {
    return "<a href='" + url + "'>" + label + "</a>";
  }

  const std::string br_tag = "<br />\n";

  std::string li_tag (const std::string &inner_html)
  {
    return "<li>" + inner_html + "</li>\n";
  }

  std::string tr_tag (const std::string &inner_html)
  {
    return "<tr>" + inner_html + "</tr>\n";
  }

  std::string td_tag (const std::string &inner_html)
  {
    return "<td>" + inner_html + "</td>";
  }

  std::string u_tag (const std::string &inner_html)
  {
    return "<u>" + inner_html + "</u>";
  }

  std::string i_tag (const std::string &inner_html)
  {
    return "<i>" + inner_html + "</i>";
  }

  std::string b_tag (const std::string &inner_html)
  {
    return "<b>" + inner_html + "</b>";
  }

  std::string red_span (const std::string &inner_html)
  {
    return "<span style=\"color: red\">" + inner_html + "</span>";
  }

  std::string ul_tag (const std::string &inner_html)
  {
    std::stringstream ss;
    ss << "<ul>" << std::endl;
    ss << inner_html;
    ss << "</ul>" << std::endl;
    return ss.str ();
  }

  std::string table_tag (const std::string &inner_html)
  {
    std::stringstream ss;
    ss << "<table>" << std::endl;
    ss << inner_html;
    ss << "</table>" << std::endl;
    return ss.str ();
  }

  void Main::handle_text_message2 (const MumbleProto::TextMessage &msg,
                                   const std::string &command,
                                   const std::string &arguments,
                                   uint32_t msg_userid)
  {
    const auto &actor = msg.actor ();
    /*
      @plugin.each do |plugin|
      plugin.handle_chat(msg, message)
      end
    */

    std::function<void(const std::string&)> reply = [this, actor] (auto msg)
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
              ca.settings["boundto"] = "nobody";
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

  CommandArgs ca = {msg, command, arguments, msg_userid, m_settings, reply,
                    *m_cli, *this
                   };
  bool boundto_msg_user = m_settings["boundto"] == std::to_string (msg_userid);

  const auto it = commands.find (command);
  if (it != std::end (commands))
    {
      if (!it->second.needs_binding || boundto_msg_user)
        {
          it->second.func (ca);
        }
    }
}

void Main::about (CommandArgs &ca)
{
  std::stringstream header;
  header << br_tag + "Hi, I am a mumble-pluginbot-plusplus bot." << br_tag;
  std::stringstream out;
  out << li_tag (a_tag (org_source_url, "Get my source code")) << std::endl;
  out << li_tag (a_tag (org_doc_url, "Read my documentation")) << std::endl;
  out << li_tag ("I am licensed under the " + a_tag (org_license_url,
                 "MIT license")) << std::endl;
  out << li_tag ("If you have any issues, bugs or ideas please tell us on " +
                 a_tag (org_issues_url, org_issues_url)) << std::endl;
  ca.reply (header.str () + ul_tag (out.str ()));
}

void Main::settings (CommandArgs &ca)
{
  std::stringstream out;
  for (const auto &it : ca.settings)
    {
      if (it.first != "logo")
        {
          out << tr_tag (td_tag (it.first) + td_tag (it.second)) << std::endl;
        }
    }
  ca.reply (table_tag (out.str ()));
}

void Main::set (CommandArgs &ca)
{
  const std::string &arguments = ca.arguments;
  auto equals_pos = arguments.find ('=');
  if (equals_pos != std::string::npos)
    {
      const std::string key = arguments.substr (0, equals_pos);
      const std::string val = arguments.substr (equals_pos + 1);
      ca.settings[key] = val;
    }
}

void Main::bind (CommandArgs &ca)
{
  if (ca.settings["boundto"] == "nobody")
    {
      ca.settings["boundto"] = std::to_string (ca.msg_userid);
    }
}

void Main::blacklist (CommandArgs &ca)
{
  const std::string &username = ca.arguments;
  Mumble::User* user = ca.cli.find_user (username);
  if (user)
    {
      const std::string hash_text = user->hash ();
      ca.settings[hash_text] = username;
      ca.reply ("This ban is active until the bot restarts. "
                "To permaban add following line to your configuration:");
      ca.reply (hash_text + "=" + username);
    }
  else
    {
      ca.reply ("User not found: " + username);
    }
}

void Main::ducking (CommandArgs &ca)
{
  const std::string ducking = ca.settings["ducking"] == "true" ? "false" : "true";
  ca.settings["ducking"] = ducking;
  if (ducking == "true")
    {
      ca.reply ("Music ducking is on.");
    }
  else
    {
      ca.reply ("Music ducking is off.");
    }
}

  void Main::duckvol (CommandArgs &ca)
  {
    if (ca.arguments == "")
      {
        const std::string &volume = ca.settings["ducking_volume"];
        const std::string &ducking = ca.settings["ducking"];
        ca.reply ("Ducking volume is set to " + volume +
                  "% of normal volume. Ducking itself it set to: " + ducking + ".");
      }
    else
      {
        try
          {
            int volume = std::stoi (ca.arguments);
            if (volume >= 0 && volume <= 100)
              {
                ca.settings["ducking_volume"] = volume;
                ca.reply ("ducking is set to " + std::to_string (volume) +
                          "% of normal volume.");
              }
            else
              {
                ca.reply ("Volume must be within a range of 0 to 100.");
              }
          }
        catch (std::invalid_argument)
          {
            ca.reply ("Invalid volume: " + ca.arguments);
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
        try
          {
            int bitrate = std::stoi (ca.arguments);
            ca.cli.bitrate (bitrate);
            ca.reply ("Encoding is set now to " + std::to_string (bitrate) + " bit/s.");
            ca.reply ("The calculated overall bandwidth is " + std::to_string (
                                                                               ca.main.overall_bandwidth ()) + " bit/s.");
          }
        catch (std::invalid_argument)
          {
            ca.reply ("Invalid bitrate: " + ca.arguments);
          }
      }
  }

  void Main::framesize (CommandArgs &ca)
  {
    if (ca.arguments == "")
      {
        std::chrono::milliseconds frame_length = ca.cli.frame_length ();
        ca.reply ("sending in " + std::to_string (frame_length.count ()) +
                  " ms frames.");
      }
    else
      {
        try
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
        catch (std::invalid_argument)
          {
            ca.reply ("Invalid framesize: " + ca.arguments);
          }
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
    /*
      @plugin.each do |plugin|
      help << plugin.name + "<br />"
      end
    */
    std::string help = br_tag + red_span ("Loaded plugins:" + br_tag + b_tag (
                                                                              ss.str ()));
    const std::string cs = ca.settings["controlstring"];
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
    const std::string cs = ca.settings["controlstring"];

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
      /*
      std::string help;
      @plugin.each do |plugin|
        help = plugin.help(help.to_s)
        ca.reply (help);
      end
      */
    }
  else if (ca.arguments != "")
    {
      // Send help for a specific plugin.
      /*
      std::string help;
      @plugin.each do |plugin|
                         help = plugin.help('') if plugin.name.upcase == message.split[1].upcase
                                                            end
                                                            ca.reply (help);
      */
    }
  else
    {
      // Send default help text.
      const std::string cs = ca.settings["controlstring"];
      std::string help = br_tag;
      help += "Hi, I am a " + a_tag (org_wiki_url, "Mumble-Ruby-Pluginbot") +
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
