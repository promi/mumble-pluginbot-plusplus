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

#include <openssl/pem.h>

#include "openssl/basic-input-output.hh"
#include "openssl/pkey/rsa.hh"
#include "openssl/x509/certificate.hh"

namespace OpenSSL
{
  class PEM
  {
  public:
    // Read RSA
    static PKey::RSA rsa_private_key (BasicInputOutput &bio);
    static PKey::RSA rsa_private_key (const std::string &pem);
    static PKey::RSA rsa_public_key (BasicInputOutput &bio);
    static PKey::RSA rsa_public_key (const std::string &pem);
    // Write RSA
    static void rsa_private_key (BasicInputOutput &bio, const PKey::RSA &rsa);
    static std::string rsa_private_key (const PKey::RSA &rsa);
    static void rsa_public_key (BasicInputOutput &bio, const PKey::RSA &rsa);
    static std::string rsa_public_key (const PKey::RSA &rsa);
    // Read X509
    static X509::Certificate x509 (BasicInputOutput &bio);
    static X509::Certificate x509 (const std::string &pem);
    // Write X509
    static void x509 (BasicInputOutput &bio, const X509::Certificate &cert);
    static std::string x509 (const X509::Certificate &cert);
  };
}
