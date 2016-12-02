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
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "openssl/basic-input-output.hh"

namespace OpenSSL
{
  class MemoryBasicInputOutput : public BasicInputOutput
  {
  public:
    MemoryBasicInputOutput ();
    std::vector<uint8_t> mem_data ();
    // TODO
    /*
      long BIO_set_mem_eof_return(BIO *b, int v);
      long BIO_set_mem_buf(BIO *b, BUF_MEM *bm, int c);
      long BIO_get_mem_ptr(BIO *b, BUF_MEM **pp);
      BIO* BIO_new_mem_buf(void *buf, int len);
    */
  };
}
