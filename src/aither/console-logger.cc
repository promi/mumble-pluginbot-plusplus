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
#include "aither/console-logger.hh"

#include <cstdarg>

namespace Aither
{
  struct ConsoleLogger::Impl
  {
    FILE *m_fp;
    Impl (FILE *fp) : m_fp (fp)
    {
    }
  };

  ConsoleLogger::ConsoleLogger (FILE *fp) : pimpl (new Impl (fp))
  {
  }

  ConsoleLogger::~ConsoleLogger ()
  {
  }

  int ConsoleLogger::printf (const char *format, ...)
  {
    va_list args;
    va_start (args, format);
    int rc = std::vfprintf (pimpl->m_fp, format, args);
    va_end (args);
    return rc;
  }

  int ConsoleLogger::print (const std::string &s)
  {
    // Don't just call printf, there might be embedded percent chars in s
    return std::fputs (s.c_str (), pimpl->m_fp);
  }

  int ConsoleLogger::print (const char *s)
  {
    // Don't just call printf, there might be embedded percent chars in s
    return std::fputs (s, pimpl->m_fp);
  }

  int ConsoleLogger::print (int i)
  {
    return printf ("%d", i);
  }
}
