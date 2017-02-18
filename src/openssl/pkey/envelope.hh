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

#include <cassert>
#include <openssl/evp.h>

#include "openssl/pkey/rsa.hh"

namespace OpenSSL
{
  namespace PKey
  {
    class Envelope
    {
    private:
      EVP_PKEY *m_evp_pkey = nullptr;
    public:
      Envelope ();
      ~Envelope ();
      Envelope (EVP_PKEY *evp_pkey);
      Envelope (const RSA &rsa);
      Envelope (Envelope &&other);
      Envelope& operator= (Envelope &&other);
      // There is no dup () function to duplicate an Envelope_PKEY struct
      // A workaround could be to use i2d_PrivateKey () and d2i_PrivateKey ()
      // which gets the DER representation and then converts it back
      // However there might only be a public key in the struct which complicates things?!
      //Envelope (const Envelope &other) = delete;
      //Envelope& operator= (Envelope other) = delete;
      // friend void swap (RSA &first, RSA &second); // nothrow
      inline const EVP_PKEY* data () const
      {
        return m_evp_pkey;
      }
    };
  }
}
