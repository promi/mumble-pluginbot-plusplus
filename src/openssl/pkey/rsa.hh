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

#include <string>

// algorithm-independent private key in memory
#include "openssl/evp.h"
// RSA algorithm
#include "openssl/rsa.h"

namespace OpenSSL::PKey
{
  class RSA
  {
  private:
    bool m_has_private = true;
    ::RSA *m_rsa = nullptr;
  public:
    RSA ();
    RSA (::RSA *rsa, bool has_private);
    RSA (size_t size);
    RSA (const RSA &other);
    RSA (RSA &&other);
    RSA& operator= (RSA other);
    friend void swap (RSA &first, RSA &second); // nothrow
    ~RSA ();
    RSA public_key ();
    inline const ::RSA* data () const
    {
      return m_rsa;
    }
  };
}
