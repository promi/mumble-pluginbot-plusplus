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

#include <string>

namespace MumblePluginBot
{
  std::string a_tag (const std::string &url, const std::string &label);
  extern const std::string br_tag;
  extern const std::string hr_tag;
  std::string li_tag (const std::string &inner_html);
  std::string tr_tag (const std::string &inner_html);
  std::string td_tag (const std::string &inner_html);
  std::string th_tag (const std::string &inner_html);
  std::string u_tag (const std::string &inner_html);
  std::string i_tag (const std::string &inner_html);
  std::string b_tag (const std::string &inner_html);
  std::string red_span (const std::string &inner_html);
  std::string red_bold_span (const std::string &inner_html);
  std::string ul_tag (const std::string &inner_html);
  std::string table_tag (const std::string &inner_html);
}
