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
#include "openssl/pkey/envelope.hh"

#include <stdexcept>

namespace OpenSSL::PKey
{
  Envelope::Envelope ()
  {
    m_evp_pkey = EVP_PKEY_new ();
    if (m_evp_pkey == nullptr)
      {
        throw std::runtime_error ("EVP_PKEY_new () failed");
      }
  }

  Envelope::~Envelope ()
  {
    EVP_PKEY_free (m_evp_pkey);
  }

  Envelope::Envelope (EVP_PKEY *evp_pkey)
  {
    m_evp_pkey = evp_pkey;
  }

  Envelope::Envelope (const RSA &rsa) : Envelope ()
  {
    if (EVP_PKEY_set1_RSA (m_evp_pkey, const_cast<::RSA*> (rsa.data ())) != 1)
      {
        throw std::runtime_error ("EVP_PKEY_set1_RSA () failed");
      }
  }

  Envelope::Envelope (Envelope &&other)
  {
    m_evp_pkey = other.m_evp_pkey;
    other.m_evp_pkey = nullptr;
  }

  Envelope& Envelope::operator= (Envelope &&other)
  {
    assert (this != &other);
    EVP_PKEY_free (m_evp_pkey);
    m_evp_pkey = other.m_evp_pkey;
    other.m_evp_pkey = nullptr;
    return *this;
  }

}
