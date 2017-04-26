/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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
#include <memory>

namespace Aither
{
  class Logger
  {
  public:
    virtual ~Logger ();
    virtual int printf (const char *format, ...) = 0;
    virtual int print (const std::string &s) = 0;
    virtual int print (const char *s) = 0;
    virtual int print (int i) = 0;
  };
  std::string get_date_string (std::chrono::system_clock::time_point t);
  std::string get_datetime_string (std::chrono::system_clock::time_point t);
}

Aither::Logger& operator<< (Aither::Logger& l, const std::string &s);
Aither::Logger& operator<< (Aither::Logger& l, const char *s);
Aither::Logger& operator<< (Aither::Logger& l, int i);


#define AITHER_DEBUG(s) \
  m_log.debug () << Aither::get_datetime_string (std::chrono::system_clock::now ()) \
  << " - " << s << "\n"

#define AITHER_ERROR(s) \
  m_log.error () << Aither::get_datetime_string (std::chrono::system_clock::now ()) \
  << " - " << s << "\n"

#define AITHER_VERBOSE(s) \
  m_log.verbose () << Aither::get_datetime_string (std::chrono::system_clock::now ()) \
  << " - " << s << "\n"

#define AITHER_WARNING(s) \
  m_log.warning () << Aither::get_datetime_string (std::chrono::system_clock::now ()) \
  << " - " << s << "\n"
