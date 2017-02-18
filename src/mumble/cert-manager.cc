/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 niko20010
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
#include <algorithm>
#include <chrono>

#include "filesystem/filesystem.hh"
#include "mumble/cert-manager.hh"
#include "io/file.hh"
#include "openssl/x509/name.hh"
#include "openssl/x509/extension-factory.hh"
#include "openssl/pem.hh"

namespace fs = FileSystem;

namespace Mumble
{
  struct CertManager::Impl
  {
    static CertificatePaths get_paths (const fs::path base_dir,
                                       const std::string &username);
    static Certificate load (const CertificatePaths &paths);
    static Certificate create (const CertificatePaths &paths,
                               const SSLCertOpts &opts, const std::string &username);
  };

  CertificatePaths CertManager::Impl::get_paths (const fs::path base_dir,
      const std::string &username)
  {
    std::string username_lower;
    std::transform (std::begin (username), std::end (username),
                    std::back_inserter (username_lower), ::tolower);

    CertificatePaths paths;
    paths.dir = base_dir / username_lower;
    auto &dir = paths.dir;
    fs::create_directories (dir);
    paths.key = dir / "private_key.pem";
    paths.cert = dir / "cert.pem";
    return paths;
  }

  Certificate CertManager::Impl::load (const CertificatePaths &paths)
  {
    Certificate container;
    container.key = std::make_unique<OpenSSL::PKey::RSA>
                    (OpenSSL::PEM::rsa_private_key (IO::File::read_all_text (paths.key)));
    container.cert = std::make_unique<OpenSSL::X509::Certificate>
                     (OpenSSL::PEM::x509 (IO::File::read_all_text (paths.cert)));
    return container;;
  }

  Certificate CertManager::Impl::create (const CertificatePaths &paths,
                                         const SSLCertOpts &opts, const std::string &username)
  {
    Certificate container;
    container.key = std::make_unique<OpenSSL::PKey::RSA> (2048);
    auto &key = *container.key;

    IO::File::write_all_text (paths.key, OpenSSL::PEM::rsa_private_key (key));

    container.cert = std::make_unique<OpenSSL::X509::Certificate> ();
    auto &cert = *container.cert;

    OpenSSL::X509::Name issuer;
    issuer.add_entry ("C", opts.country_code);
    issuer.add_entry ("O", opts.organization);
    issuer.add_entry ("OU", opts.organization_unit);
    issuer.add_entry ("CN", username);

    cert.issuer (issuer);
    cert.subject (issuer);
    cert.not_before (0);
    cert.not_after (5 * 365 * 24 * 60 * 60);
    cert.public_key (OpenSSL::PKey::Envelope (key.public_key ()));
    srand (time(0));
    cert.serial (rand() % 65536 + 1);
    cert.version (2);

    OpenSSL::X509::ExtensionFactory ef { cert, cert };
    cert.add_extension (ef.create_extension ("basicConstraints", "CA:TRUE", true));
    cert.add_extension (ef.create_extension ("keyUsage", "keyCertSign, cRLSign",
                        true));
    cert.add_extension (ef.create_extension ("subjectKeyIdentifier", "hash",
                        false));
    cert.add_extension (ef.create_extension ("authorityKeyIdentifier",
                        "keyid:always", false));

    cert.sign (key, OpenSSL::PKey::EnvelopeMessageDigest::sha256 ());
    IO::File::write_all_text (paths.cert, OpenSSL::PEM::x509 (cert));
    return container;
  }

  std::pair<CertificatePaths, Certificate> CertManager::get_certificate (
    SSLCertOpts opts, const std::string &username)
  {
    auto paths = Impl::get_paths (opts.cert_dir, username);
    if (fs::exists (paths.key) && fs::exists (paths.cert))
      {
        return std::make_pair (paths, Impl::load (paths));
      }
    else
      {
        return std::make_pair (paths, Impl::create (paths, opts, username));
      }
  }
}
