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
#include "openssl/basic-input-output.hh"

namespace OpenSSL
{
  BasicInputOutput::BasicInputOutput ()
  {
  }

  BasicInputOutput::~BasicInputOutput ()
  {
    BIO_free (m_bio);
  }

  void BasicInputOutput::write (const std::vector<uint8_t> &data)
  {
    BIO_write (m_bio, data.data (), data.size ());
  }
}
