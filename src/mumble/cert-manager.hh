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
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    CertManager (const std::string &username, SSLCertOpts opts);
    ~CertManager ();
    const Certificate& cert () const;
    const std::experimental::filesystem::path& cert_dir_path () const;
    const std::experimental::filesystem::path& private_key_path () const;
    const std::experimental::filesystem::path& public_key_path () const;
    const std::experimental::filesystem::path& cert_path () const;
  };
}
