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
    m_rsa = RSA_new ();
    if (m_rsa == nullptr)
      {
        throw std::runtime_error ("RSA_new () failed");
      }
  }

  RSA::RSA (::RSA *rsa, bool has_private)
    : m_has_private (has_private), m_rsa (rsa)
  {
  }

  RSA::RSA (size_t size) : RSA ()
  {
    // See also http://stackoverflow.com/a/30493975/426242
    using BN_ptr = std::unique_ptr<BIGNUM, decltype(&::BN_free)>;
    BN_ptr bn {BN_new (), ::BN_free};
    if (bn == nullptr)
      {
        throw std::runtime_error ("BN_new () failed");
      }
    if (BN_set_word (bn.get (), RSA_F4) != 1)
      {
        throw std::runtime_error ("BN_set_word () failed");
      }
    if (RSA_generate_key_ex (m_rsa, size, bn.get (), nullptr) != 1)
      {
        throw std::runtime_error ("RSA_generate_key_ex () failed");
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
            throw std::runtime_error ("RSAPrivateKey_dup () failed");
          }
      }
    else
      {
        m_rsa = RSAPublicKey_dup (const_cast<::RSA*> (other.m_rsa));
        if (m_rsa == nullptr)
          {
            throw std::runtime_error ("RSAPublicKey_dup () failed");
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
        throw std::runtime_error ("RSAPublicKey_dup () failed");
      }
    return RSA (rsa, false);
  }
}
