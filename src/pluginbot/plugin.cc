/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
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
#include <pluginbot/plugin.hh>

namespace MumblePluginBot
{
  void Plugin::ticks (std::chrono::time_point<std::chrono::system_clock>
                      time_point)
  {
    (void) time_point;
  }

  std::string Plugin::help ()
  {
    return "no help text available for plugin: " + name ();
  }
/*
class Plugin
  def self.plugins
    @plugins ||= []
  end

  def self.inherited(klass)
    @plugins ||= []

    @plugins << klass
  end

  # Usually a good idea for debugging if you have lots of methods
  def handle_chat(msg, message)
    @user = msg.actor
    #raise "#{self.class.name} doesn't implement `handle_chat`!"
  end

  def handle_command(command)
    #raise "#{self.class.name} doesn't implement `handle_command`!"
  end

  def handle_response
    #
  end

  def init(init)
    @@bot = init
  end

  private
  def prozessmessage(message)
    # count lines
    # for future use (send long messages in smaller parts)
    lines = message.count("<br>") + message.count("<tr>")
    puts lines
    return message
  end
  def privatemessage(message)
    begin
      @@bot[:cli].text_user(@user, message)
    rescue
      puts "Sending message to user #{@user} failed. Maybe left server before we try to send."
    end
  end
  def messageto(actor, message)
    begin
      @@bot[:cli].text_user(actor, message)
    rescue
      puts "Sending message to user #{actor} failed. Maybe left server before we try to send."
    end
  end
  def channelmessage(message)
    begin
      @@bot[:cli].text_channel(@@bot[:cli].me.current_channel, message)
    rescue
      puts "Sending message to channel #{@@bot[:cli].me.current_channel} failed. ->should never happen<-"
    end
  end

end

*/
}
