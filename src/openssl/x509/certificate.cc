/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2016 Phobos (promi) <prometheus@unterderbruecke.de>
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
#include "openssl/x509/certificate.hh"

namespace OpenSSL::X509
{
  Certificate::Certificate ()
  {
    m_x509 = X509_new ();
    if (m_x509 == nullptr)
      {
        throw std::runtime_error ("X509_new () failed");
      }
  }

  Certificate::Certificate (::X509 *x509) : m_x509 (x509)
  {
  }

  Certificate::Certificate(const Certificate &other)
  {
    m_x509 = X509_dup (other.m_x509);
    if (m_x509 == nullptr)
      {
        throw std::runtime_error ("X509_dup () failed");
      }
  }

  Certificate::~Certificate ()
  {
    X509_free (m_x509);
  }

  void Certificate::issuer (const Name &issuer)
  {
    X509_set_issuer_name (m_x509, const_cast<X509_NAME*> (issuer.data ()));
  }

  Name Certificate::issuer (void) const
  {
    return Name (X509_get_issuer_name (m_x509));
  }

  void Certificate::subject (const Name &subject)
  {
    X509_set_subject_name (m_x509, const_cast<X509_NAME*> (subject.data ()));
  }

  void Certificate::not_before (long not_before)
  {
    X509_gmtime_adj (X509_get_notBefore (m_x509), not_before);
  }

  void Certificate::not_after (time_t not_after)
  {
    X509_gmtime_adj (X509_get_notAfter (m_x509), not_after);
  }

  void Certificate::public_key (OpenSSL::PKey::Envelope &&public_key)
  {
    m_public_key = std::move (public_key);
    X509_set_pubkey (m_x509, const_cast<EVP_PKEY*> (m_public_key.data ()));
  }

  void Certificate::serial (int serial)
  {
    ASN1_INTEGER_set (X509_get_serialNumber (m_x509), serial);
  }

  void Certificate::version (long version)
  {
    if (X509_set_version (m_x509, version) != 1)
      {
        throw std::runtime_error ("X509_set_version () failed");
      }
  }

  void Certificate::add_extension (const Extension &extension)
  {
    if (X509_add_ext (m_x509, const_cast<X509_EXTENSION*> (extension.data ()),
                      -1) != 1)
      {
        throw std::runtime_error ("X509_add_ext () failed");
      }
  }

  void Certificate::sign (const OpenSSL::PKey::Envelope &key,
                          const OpenSSL::PKey::EnvelopeMessageDigest &digest)
  {
    // C-C-C-Combo Breaker: X509_sign returns the size of the signature
    // in bytes, instead of "1" for success, like most other functions do.
    // It always returns 0 on failure though.
    if (X509_sign (m_x509, const_cast<EVP_PKEY*> (key.data ()),
                   digest.data ()) == 0)
      {
        throw std::runtime_error ("X509_sign () failed");
      }
  }
}
