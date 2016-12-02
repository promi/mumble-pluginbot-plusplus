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

#include "openssl/pkey/rsa.hh"
#include "openssl/x509/certificate.hh"
#include "openssl/ssl/method.hh"

namespace OpenSSL::SSL
{
  class Context
  {
  private:
    SSL_CTX *m_ssl_ctx;
    std::unique_ptr<OpenSSL::PKey::RSA> m_key;
    std::unique_ptr<OpenSSL::X509::Certificate> m_cert;
  public:
    Context (Method method);
    ~Context ();
    void verify_none ();
    void key (const OpenSSL::PKey::RSA &key);
    void cert (const OpenSSL::X509::Certificate &cert);
    inline auto data () const
    {
      return m_ssl_ctx;
    }
  };
}
