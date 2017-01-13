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
#include "name.hh"

#include <stdexcept>

namespace OpenSSL::X509
{
  Name::Name ()
  {
    m_name = X509_NAME_new ();
    if (m_name == nullptr)
      {
        throw std::runtime_error ("X509_NAME_new () failed");
      }
  }

  Name::Name (X509_NAME *name) : m_name (name)
  {
  }

  Name::~Name ()
  {
    X509_NAME_free (m_name);
  }

  void Name::add_entry (const std::string &field, const std::string &text)
  {
    if (field == "C" && text.size () > 2)
      {
        throw std::runtime_error ("Country name too long (should be 2 characters long)");
      }
    if (X509_NAME_add_entry_by_txt (m_name, field.c_str (), MBSTRING_UTF8,
                                    reinterpret_cast<const unsigned char*> (text.c_str ()),
                                    text.size (), -1, 0) != 1)
      {
        throw std::runtime_error ("X509_NAME_add_entry_by_txt () failed");
      }
  }
}
