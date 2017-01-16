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
#include <pluginbot/html.hh>

#include <sstream>

namespace MumblePluginBot
{
  std::string a_tag (const std::string &url, const std::string &label)
  {
    return "<a href='" + url + "'>" + label + "</a>";
  }

  const std::string br_tag = "<br />\n";

  const std::string hr_tag = "<hr />\n";

  std::string li_tag (const std::string &inner_html)
  {
    return "<li>" + inner_html + "</li>\n";
  }

  std::string tr_tag (const std::string &inner_html)
  {
    return "<tr>" + inner_html + "</tr>\n";
  }

  std::string td_tag (const std::string &inner_html)
  {
    return "<td>" + inner_html + "</td>";
  }

  std::string u_tag (const std::string &inner_html)
  {
    return "<u>" + inner_html + "</u>";
  }

  std::string i_tag (const std::string &inner_html)
  {
    return "<i>" + inner_html + "</i>";
  }

  std::string b_tag (const std::string &inner_html)
  {
    return "<b>" + inner_html + "</b>";
  }

  std::string red_span (const std::string &inner_html)
  {
    return "<span style=\"color: red\">" + inner_html + "</span>";
  }

  std::string red_bold_span (const std::string &inner_html)
  {
    return "<span style=\"color: red; font-weight:bold\">" + inner_html + "</span>";
  }

  std::string ul_tag (const std::string &inner_html)
  {
    return "<ul>\n" + inner_html + "\n</ul>\n";
  }

  std::string table_tag (const std::string &inner_html)
  {
    return "<table>\n" + inner_html + "\n</table>\n";
  }
}
