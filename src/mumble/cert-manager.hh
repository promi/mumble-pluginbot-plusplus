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
#pragma once

#include <memory>
#include <string>
#include <experimental/filesystem>

#include "mumble/certificate.hh"
#include "mumble/configuration.hh"

namespace Mumble
{
  class CertManager
  {
  private:
    std::string m_username;
    SSLCertOpts m_opts;
    Certificate m_certi;
    std::experimental::filesystem::path m_cert_dir;
    std::experimental::filesystem::path m_private_key_path;
    std::experimental::filesystem::path m_public_key_path;
    std::experimental::filesystem::path m_cert_path;
  public:
    CertManager (const std::string &username, SSLCertOpts opts);
    inline const OpenSSL::PKey::RSA& key () const
    {
      if (m_certi.key == nullptr)
        {
          throw std::string ("key not initialized");
        }
      return *m_certi.key;
    }
    inline const OpenSSL::X509::Certificate& cert () const
    {
      if (m_certi.cert == nullptr)
        {
          throw std::string ("cert not initialized");
        }
      return *m_certi.cert;
    }
    inline const Certificate& certi () const
    {
      return m_certi;
    }
    inline const std::experimental::filesystem::path& private_key_path () const
    {
      return m_private_key_path;
    }
    inline const std::experimental::filesystem::path& public_key_path () const
    {
      return m_public_key_path;
    }
    inline const std::experimental::filesystem::path& cert_path () const
    {
      return m_cert_path;
    }
  private:
    void setup_key ();
    void setup_cert ();
  };
}
