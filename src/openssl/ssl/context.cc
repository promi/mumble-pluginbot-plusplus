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
#include "openssl/ssl/context.hh"

namespace OpenSSL::SSL
{
  Context::Context (Method method)
  {
    m_ssl_ctx = SSL_CTX_new (method.data ());
    if (m_ssl_ctx == nullptr)
      {
        throw std::runtime_error ("SSL_CTX_new () failed");
      }
  }

  Context::~Context ()
  {
    SSL_CTX_free (m_ssl_ctx);
  }

  void Context::verify_none ()
  {
    SSL_CTX_set_verify (m_ssl_ctx, SSL_VERIFY_NONE, nullptr);
  }

  void Context::key (const OpenSSL::PKey::RSA &key)
  {
    if (SSL_CTX_use_RSAPrivateKey (m_ssl_ctx, const_cast<RSA*> (key.data ())) != 1)
      {
        throw std::runtime_error ("SSL_CTX_use_RSAPrivateKey () failed");
      }
  }

  void Context::cert (const OpenSSL::X509::Certificate &cert)
  {
    if (SSL_CTX_use_certificate (m_ssl_ctx,
                                 const_cast<::X509*> (cert.data ())) != 1)
      {
        throw std::runtime_error ("SSL_CTX_use_certificate () failed");
      }
  }
}
