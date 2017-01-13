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
#include "openssl/pem.hh"
#include "openssl/memory-basic-input-output.hh"

namespace OpenSSL
{
  PKey::RSA PEM::rsa_private_key (BasicInputOutput &bio)
  {
    ::RSA *rsa = PEM_read_bio_RSAPrivateKey (bio.data (), nullptr, nullptr,
                 nullptr);
    if (rsa == nullptr)
      {
        throw std::runtime_error ("PEM_read_bio_RSAPrivateKey () failed");
      }
    return PKey::RSA (rsa, true);
  }

  PKey::RSA PEM::rsa_private_key (const std::string &pem)
  {
    OpenSSL::MemoryBasicInputOutput bio;
    bio.write (std::vector<uint8_t> (std::begin (pem), std::end (pem)));
    return rsa_private_key (bio);
  }

  PKey::RSA PEM::rsa_public_key (BasicInputOutput &bio)
  {
    ::RSA *rsa = PEM_read_bio_RSAPublicKey (bio.data (), nullptr, nullptr, nullptr);
    if (rsa == nullptr)
      {
        throw std::runtime_error ("PEM_read_bio_RSAPublicKey () failed");
      }
    return PKey::RSA (rsa, false);
  }

  PKey::RSA PEM::rsa_public_key (const std::string &pem)
  {
    MemoryBasicInputOutput bio;
    bio.write (std::vector<uint8_t> (std::begin (pem), std::end (pem)));
    return rsa_public_key (bio);
  }

  void PEM::rsa_private_key (BasicInputOutput &bio, const PKey::RSA &rsa)
  {
    // TODO: Why does the PEM function take a non const pointer here,
    //       but not in the corresponding PublicKey version???
    if (PEM_write_bio_RSAPrivateKey (bio.data (), const_cast<RSA*> (rsa.data ()),
                                     nullptr, nullptr, 0, nullptr, nullptr) != 1)
      {
        throw std::runtime_error ("PEM_write_bio_RSAPrivateKey () failed");
      }
  }

  std::string PEM::rsa_private_key (const PKey::RSA &rsa)
  {
    MemoryBasicInputOutput bio;
    rsa_private_key (bio, rsa);
    auto data = bio.mem_data ();
    return std::string (std::begin (data), std::end (data));
  }

  void PEM::rsa_public_key (BasicInputOutput &bio, const PKey::RSA &rsa)
  {
    if (PEM_write_bio_RSAPublicKey (bio.data (), rsa.data ()) != 1)
      {
        throw std::runtime_error ("PEM_write_bio_RSAPublicKey () failed");
      }
  }

  std::string PEM::rsa_public_key (const PKey::RSA &rsa)
  {
    MemoryBasicInputOutput bio;
    rsa_public_key (bio, rsa);
    auto data = bio.mem_data ();
    return std::string (std::begin (data), std::end (data));
  }

  X509::Certificate PEM::x509 (BasicInputOutput &bio)
  {
    ::X509 *x509 = PEM_read_bio_X509 (bio.data (), nullptr, nullptr, nullptr);
    if (x509 == nullptr)
      {
        throw std::runtime_error ("PEM_read_bio_X509 () failed");
      }
    return X509::Certificate (x509);
  }

  X509::Certificate PEM::x509 (const std::string &pem)
  {
    MemoryBasicInputOutput bio;
    bio.write (std::vector<uint8_t> (std::begin (pem), std::end (pem)));
    return x509 (bio);
  }

  void PEM::x509 (BasicInputOutput &bio, const X509::Certificate &cert)
  {
    if (PEM_write_bio_X509 (bio.data (), const_cast<::X509*> (cert.data ())) != 1)
      {
        throw std::runtime_error ("PEM_write_bio_X509 () failed");
      }
  }

  std::string PEM::x509 (const X509::Certificate &cert)
  {
    MemoryBasicInputOutput bio;
    x509 (bio, cert);
    auto data = bio.mem_data ();
    return std::string (std::begin (data), std::end (data));
  }
}
