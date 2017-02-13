/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 niko20010
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
#include <algorithm>
#include <chrono>

#include "mumble/cert-manager.hh"
#include "io/file.hh"
#include "openssl/x509/name.hh"
#include "openssl/x509/extension-factory.hh"
#include "openssl/pem.hh"

namespace fs = std::experimental::filesystem;

namespace Mumble
{
  struct CertManager::Impl
  {
    std::string m_username;
    SSLCertOpts m_opts;
    Certificate m_certi;
    fs::path m_cert_dir;
    fs::path m_private_key_path;
    fs::path m_public_key_path;
    fs::path m_cert_path;
    Impl (const std::string &username, SSLCertOpts opts);
    void setup_key ();
    void setup_cert ();
  };

  CertManager::CertManager (const std::string &username, SSLCertOpts opts)
    : pimpl (std::make_unique<Impl> (username, opts))
  {
  }

  CertManager::~CertManager ()
  {
  }
  
  const Certificate& CertManager::cert () const
  {
    return pimpl->m_certi;
  }

  const fs::path& CertManager::cert_dir_path () const
  {
    return pimpl->m_cert_dir;
  }
  
  const fs::path& CertManager::private_key_path () const
  {
    return pimpl->m_private_key_path;
  }

  const fs::path& CertManager::public_key_path () const
  {
    return pimpl->m_public_key_path;
  }

  const fs::path& CertManager::cert_path () const
  {
    return pimpl->m_cert_path;
  }

  CertManager::Impl::Impl (const std::string &username, SSLCertOpts opts)
      : m_username (username), m_opts (opts), m_cert_dir (opts.cert_dir)
    {
      std::string username_lower;
      std::transform (std::begin (username), std::end (username),
                      std::back_inserter (username_lower), ::tolower);
      m_cert_dir /= username_lower;
      fs::create_directories (m_cert_dir);
      m_private_key_path = m_cert_dir / "private_key.pem";
      m_public_key_path = m_cert_dir / "public_key.pem";
      m_cert_path = m_cert_dir / "cert.pem";
      setup_key ();
      setup_cert ();
    }

  void CertManager::Impl::setup_key ()
  {
    if (fs::exists (m_private_key_path))
      {
        m_certi.key = std::make_unique<OpenSSL::PKey::RSA> (
                      OpenSSL::PEM::rsa_private_key (IO::File::read_all_text (m_private_key_path)));
      }
    else
      {
        m_certi.key = std::make_unique<OpenSSL::PKey::RSA> (2048);
        auto &key = *m_certi.key;
        IO::File::write_all_text (m_private_key_path,
                                  OpenSSL::PEM::rsa_private_key (key));
        IO::File::write_all_text (m_public_key_path,
                                  OpenSSL::PEM::rsa_public_key (key));
      }
  }

  void CertManager::Impl::setup_cert ()
  {
    if (fs::exists (m_cert_path))
      {
        m_certi.cert = std::make_unique<OpenSSL::X509::Certificate> (OpenSSL::PEM::x509 (
                   IO::File::read_all_text (m_cert_path)));
      }
    else
      {
        OpenSSL::X509::Name issuer;
        issuer.add_entry ("C", m_opts.country_code);
        issuer.add_entry ("O", m_opts.organization);
        issuer.add_entry ("OU", m_opts.organization_unit);
        issuer.add_entry ("CN", m_username);
        m_certi.cert = std::make_unique<OpenSSL::X509::Certificate> ();
        auto &cert = *m_certi.cert;
        auto &key = *m_certi.key;
        cert.issuer (issuer);
        cert.subject (issuer);
        cert.not_before (0);
        cert.not_after (5 * 365 * 24 * 60 * 60);
        cert.public_key (OpenSSL::PKey::Envelope (key.public_key ()));
        srand (time(0));
        cert.serial (rand() % 65536 + 1);
        cert.version (2);

        OpenSSL::X509::ExtensionFactory ef { cert, cert };

        cert.add_extension (ef.create_extension ("basicConstraints", "CA:TRUE",
                               true));
        cert.add_extension (ef.create_extension ("keyUsage", "keyCertSign, cRLSign",
                               true));
        cert.add_extension (ef.create_extension ("subjectKeyIdentifier", "hash",
                               false));
        cert.add_extension (ef.create_extension ("authorityKeyIdentifier",
                               "keyid:always", false));

        cert.sign (key, OpenSSL::PKey::EnvelopeMessageDigest::sha256 ());
        IO::File::write_all_text (m_cert_path, OpenSSL::PEM::x509 (cert));
      }
  }
}
