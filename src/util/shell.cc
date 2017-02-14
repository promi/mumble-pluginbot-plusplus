/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 Natenom
    Copyright (c) 2015 netinetwalker
    Copyright (c) 2015 loscoala
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
#include "util/shell.hh"

#include <cstdio>

#include "util/string.hh"

namespace StringUtils = Util::String;

namespace Util::Shell
{
  std::string squote (const std::string &s)
  {
    std::string result = "'";
    for (const char &c : s)
      {
        if (c == '\'')
          {
            result += "'\\''";
          }
        else
          {
            result += c;
          }
      }
    return result + "'";
  };

  std::string exec (const std::string &cmd)
  {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, int(*)(FILE*)> pipe (popen (cmd.c_str (), "r"), pclose);
    if (pipe == nullptr)
      {
        throw std::runtime_error ("popen() failed");
      }
    while ((fgets (buffer.data (), buffer.size (), pipe.get ())) != nullptr)
      {
        result += buffer.data ();
      }
    return result;
  }

  std::string nice_exec (const std::string &cmd)
  {
    return Shell::exec (StringUtils::intercalate ({"nice", "-n20", cmd}));
  }
}
