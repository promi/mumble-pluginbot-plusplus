/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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

#include <string>
#include <memory>

namespace FileSystem
{
  class path
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    path ();
    path (const std::string &s);
    path (const path &other);
    path (path &&other);
    ~path ();
    path filename () const;
    std::string extension () const;
    std::string string () const;
    bool is_absolute () const;
    path& operator=(const path& p);
    path& operator=(path& p);
  };

  path operator/(const path &lhs, const path &rhs);
  path operator/(const path &lhs, const std::string &rhs);

  void create_directories (const path &dir);
  bool exists (const path &path);
}
