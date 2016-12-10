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
  }                                                                   \

#define STRING_GETTER(setting) [&] () { return setting; }
#define STRING_SETTER(setting) [&] (const std::string& value) { setting = value; }

#define BOOL(setting) BOOL_GETTER(setting), BOOL_SETTER(setting)
#define STRING(setting) STRING_GETTER(setting), STRING_SETTER(setting)
#define PATH(setting) STRING(setting)

namespace MumblePluginBot
{
  std::list<SettingDescriptor> SettingDescriptors::create (Settings &settings)
  {
    std::list<SettingDescriptor> l;

    l.push_back ({"", "version", STRING(settings.version)});
    l.push_back ({"", "main_tempdir", PATH(settings.main_tempdir)});
    l.push_back ({"", "ducking", BOOL(settings.ducking)});
    return l;
  }
}
