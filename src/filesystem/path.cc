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
#include "filesystem/filesystem.hh"

#include <stdexcept>
#include <sstream>
#include <sys/stat.h>

namespace FileSystem
{
  struct path::Impl
  {
    std::string m_data;
    Impl (const std::string &data) : m_data (data)
    {
    }
  };

  path::path () : pimpl (std::make_unique<Impl> (""))
  {
  }

  path::path (const std::string &s) : pimpl (std::make_unique<Impl> (s))
  {
  }

  path::path (const path &other) : pimpl (std::make_unique<Impl>
                                            (other.pimpl->m_data))
  {
  }

  path::path (path &&other) : pimpl (std::make_unique<Impl> (std::move (
                                         other.pimpl->m_data)))
  {
  }

  path::~path ()
  {
  }

  path path::filename () const
  {
    auto const &s = pimpl->m_data;
    auto const pos = s.find_last_of ('/');
    if (pos == std::string::npos)
      {
        return path {s};
      }
    else
      {
        return path {s.substr (pos + 1)};
      }
  }

  std::string path::extension () const
  {
    auto const &s = pimpl->m_data;
    auto const pos = s.find_last_of ('.');
    return s.substr (pos);
  }

  std::string path::string () const
  {
    return pimpl->m_data;
  }

  bool path::is_absolute () const
  {
    return pimpl->m_data.find ('/') == 0;
  }

  path& path::operator=(const path& p)
  {
    pimpl->m_data = p.pimpl->m_data;
    return *this;
  }

  path& path::operator=(path& p)
  {
    pimpl->m_data = p.pimpl->m_data;
    return *this;
  }

  path operator/(const path &lhs, const path &rhs)
  {
    return path { lhs.string () + '/' + rhs.string () };
  }

  path operator/(const path &lhs, const std::string &rhs)
  {
    return path { lhs.string () + '/' + rhs };
  }

  void create_directories (const path &dir)
  {
    std::stringstream ss {dir.string ()};
    path current {};
    for (std::string part; std::getline (ss, part, '/'); )
      {
        if (part == "")
          {
            current = path {current.string () + "/"};
          }
        else
          {
            current = path {current.string () + part + "/"};
          }
        if (!exists (current))
          {
            if (mkdir (current.string ().c_str (), 0777) != 0)
              {
                throw std::runtime_error ("mkdir () failed");
              }
          }
      }
  }

  bool exists (const path &path)
  {
    struct stat stbuf;
    return stat (path.string ().c_str (), &stbuf) == 0;
  }
}
