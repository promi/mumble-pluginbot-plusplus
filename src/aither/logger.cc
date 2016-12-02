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
#include "aither/logger.hh"

namespace Aither
{
  Logger::~Logger ()
  {
  }

  std::string internal_get_date_string (const std::string &fmt,
                                        std::chrono::system_clock::time_point t)
  {
    char some_buffer[100];
    auto as_time_t = std::chrono::system_clock::to_time_t (t);
    struct tm tm;
    if (::gmtime_r (&as_time_t, &tm))
      {
        if (std::strftime (some_buffer, sizeof(some_buffer), fmt.c_str (), &tm))
          {
            return std::string {some_buffer};
          }
      }
    throw std::runtime_error ("Failed to get current date as string");
  }

  std::string get_date_string (std::chrono::system_clock::time_point t)
  {
    return internal_get_date_string ("%F", t);
  }

  std::string get_datetime_string (std::chrono::system_clock::time_point t)
  {
    return internal_get_date_string ("%F %T", t);
  }
}

Aither::Logger& operator<< (Aither::Logger& l, const std::string &s)
{
  l.printf (s.c_str ());
  return l;
}

Aither::Logger& operator<< (Aither::Logger& l, const char *s)
{
  l.printf (s);
  return l;
}
