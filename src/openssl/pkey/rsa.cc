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
#include "openssl/pkey/rsa.hh"
#include "openssl/pem.hh"

namespace OpenSSL::PKey
{
  RSA::RSA ()
  {
  }

  RSA::RSA (::RSA *rsa, bool has_private)
    : m_has_private (has_private), m_rsa (rsa)
  {
  }

  RSA::RSA (size_t size) : RSA ()
  {
    m_rsa = RSA_generate_key (size, RSA_F4, nullptr, nullptr);
    if (m_rsa == nullptr)
      {
        throw std::string ("RSA_generate_key () failed");
      }
  }

  RSA::RSA (const RSA &other)
    : m_has_private (other.m_has_private)
  {
    if (other.m_has_private)
      {
        m_rsa = RSAPrivateKey_dup (const_cast<::RSA*> (other.m_rsa));
        if (m_rsa == nullptr)
          {
            throw std::string ("RSAPrivateKey_dup () failed");
          }
      }
    else
      {
        m_rsa = RSAPublicKey_dup (const_cast<::RSA*> (other.m_rsa));
        if (m_rsa == nullptr)
          {
            throw std::string ("RSAPublicKey_dup () failed");
          }
      }
  }

  RSA::RSA (RSA &&other) : RSA ()
  {
    swap (*this, other);
  }

  RSA& RSA::operator= (RSA other)
  {
    swap (*this, other);
    return *this;
  }

  void swap (RSA &first, RSA &second)
  {
    using std::swap;
    swap (first.m_has_private, second.m_has_private);
    swap (first.m_rsa, second.m_rsa);
  }

  RSA::~RSA ()
  {
    RSA_free (m_rsa);
  }

  RSA RSA::public_key ()
  {
    ::RSA *rsa = RSAPublicKey_dup (m_rsa);
    if (rsa == nullptr)
      {
        throw std::string ("RSAPublicKey_dup () failed");
      }
    return RSA (rsa, false);
  }
}
