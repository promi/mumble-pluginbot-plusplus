/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
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
#include "openssl/memory-basic-input-output.hh"

namespace OpenSSL
{
  MemoryBasicInputOutput::MemoryBasicInputOutput ()
  {
    m_bio = BIO_new (BIO_s_mem ());
    if (m_bio == nullptr)
      {
        throw std::string ("BIO_new () failed");
      }
  }

  std::vector<uint8_t> MemoryBasicInputOutput::mem_data ()
  {
    char *data;
    auto len = BIO_get_mem_data (m_bio, &data);
    return std::vector<uint8_t> (data, data + len);
  }
}
