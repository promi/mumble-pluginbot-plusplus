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
#pragma once

#include <memory>

#include "aither/log-severity.hh"
#include "aither/logger.hh"

namespace Aither
{
  class Log
  {
  public:
    Log (LogSeverity minimum_severity);
    ~Log ();
    Logger& debug () const;
    Logger& verbose () const;
    Logger& warning () const;
    Logger& error () const;
  private:
    struct Impl;
    const std::unique_ptr<Impl> pimpl;
  };
}
