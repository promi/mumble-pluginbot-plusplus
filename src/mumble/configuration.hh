/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2012 Matthew Perry (mattvperry)
    Copyright (c) 2013 Matthew Perry (mattvperry)
    Copyright (c) 2014 Aaron Herting (qwertos)
    Copyright (c) 2014 Jack Chen (chendo)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
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
#pragma once

#include <string>

#include "filesystem/filesystem.hh"

namespace Mumble
{
  struct SSLCertOpts
  {
    FileSystem::path cert_dir;
    std::string country_code = "US";
    std::string organization = "github.com";
    std::string organization_unit = "Engineering";
  };

  struct Configuration
  {
    std::string host;
    int port = 64738;
    std::string username;
    std::string password;
    int sample_rate = 48000;
    int bitrate = 32000;
    bool vbr_rate = false;
    SSLCertOpts ssl_cert_opts;
  };
}
