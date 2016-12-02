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

#include <memory>
#include <openssl/x509.h>

#include "openssl/pkey/envelope.hh"
#include "openssl/pkey/envelope-message-digest.hh"
#include "openssl/x509/name.hh"
#include "openssl/x509/extension.hh"

namespace OpenSSL::X509
{
  class Certificate
  {
  private:
    ::X509 *m_x509;
    OpenSSL::PKey::Envelope m_public_key;
  public:
    Certificate ();
    Certificate (::X509 *x509);
    Certificate (const Certificate &other);
    ~Certificate ();
    void issuer (const Name &issuer);
    Name issuer (void) const;
    void subject (const Name &subject);
    void not_before (long not_before);
    void not_after (long not_after);
    void public_key (OpenSSL::PKey::Envelope &&public_key);
    void serial (int serial);
    void version (long version);
    void add_extension (const Extension &extension);
    void sign (const OpenSSL::PKey::Envelope &key,
               const OpenSSL::PKey::EnvelopeMessageDigest &digest);
    inline const ::X509* data () const
    {
      return m_x509;
    }
  };
}
