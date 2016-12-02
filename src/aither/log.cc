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

#include "aither/log.hh"
#include "aither/console-logger.hh"
#include "aither/void-logger.hh"

namespace Aither
{
  struct Log::Impl
  {
    LogSeverity m_minimum_severity;
    std::unique_ptr<Logger> m_debug;
    std::unique_ptr<Logger> m_verbose;
    std::unique_ptr<Logger> m_warning;
    std::unique_ptr<Logger> m_error;

    Impl (LogSeverity minimum_severity)
    {
      m_minimum_severity  = minimum_severity;
      switch (minimum_severity)
        {
        case LogSeverity::Debug:
          m_debug = std::make_unique<ConsoleLogger> (stdout);
          m_verbose = std::make_unique<ConsoleLogger> (stdout);
          m_warning = std::make_unique<ConsoleLogger> (stdout);
          m_error = std::make_unique<ConsoleLogger> (stderr);
          break;

        case LogSeverity::Verbose:
          m_debug = std::make_unique<VoidLogger> ();
          m_verbose = std::make_unique<ConsoleLogger> (stdout);
          m_warning = std::make_unique<ConsoleLogger> (stdout);
          m_error = std::make_unique<ConsoleLogger> (stderr);
          break;

        case LogSeverity::Warning:
          m_debug = std::make_unique<VoidLogger> ();
          m_verbose = std::make_unique<VoidLogger> ();
          m_warning = std::make_unique<ConsoleLogger> (stdout);
          m_error = std::make_unique<ConsoleLogger> (stderr);
          break;

        case LogSeverity::Error:
          m_debug = std::make_unique<VoidLogger> ();
          m_verbose = std::make_unique<VoidLogger> ();
          m_warning = std::make_unique<VoidLogger> ();
          m_error = std::make_unique<ConsoleLogger> (stderr);
          break;
        }
    }
  };

  Log::Log (LogSeverity minimum_severity) : pimpl (new Impl (minimum_severity))
  {
  }

  Log::~Log ()
  {
  }

  Logger& Log::debug () const
  {
    return *pimpl->m_debug;
  }

  Logger& Log::verbose () const
  {
    return *pimpl->m_verbose;
  }

  Logger& Log::warning () const
  {
    return *pimpl->m_warning;
  }

  Logger& Log::error () const
  {
    return *pimpl->m_error;
  }
}
