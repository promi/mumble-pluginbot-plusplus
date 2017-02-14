/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 Natenom
    Copyright (c) 2015 netinetwalker
    Copyright (c) 2015 loscoala
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
#include "util/image-magick.hh"

#include "util/shell.hh"
#include "util/string.hh"

namespace Shell = Util::Shell;
namespace StringUtils = Util::String;

namespace Util
{
  void ImageMagick::resize (const std::experimental::filesystem::path &from,
                            const std::string &size, const std::experimental::filesystem::path &to)
  {
    Shell::nice_exec (StringUtils::intercalate (
    {
      "convert",
      Shell::squote (from),
      "-resize " + Shell::squote (size),
      Shell::squote (to)
    }));
  }
}
