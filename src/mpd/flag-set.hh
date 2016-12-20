/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2015 Omair
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

// This code is based on Omairs SO answer here:
// https://stackoverflow.com/a/31906371/426242

#include <type_traits>
#include <limits>
#include <bitset>

template <typename TENUM>
class FlagSet
{
private:
  using TUNDER = typename std::underlying_type<TENUM>::type;
  std::bitset<std::numeric_limits<TUNDER>::max()> m_flags;
public:
  FlagSet() = default;

  template <typename... ARGS>
  FlagSet(TENUM f, ARGS... args) : FlagSet(args...)
  {
    set(f);
  }
  FlagSet& set(TENUM f)
  {
    m_flags.set(static_cast<TUNDER>(f));
    return *this;
  }
  bool test(TENUM f)
  {
    return m_flags.test(static_cast<TUNDER>(f));
  }
  FlagSet& operator |= (TENUM f)
  {
    return set(f);
  }
};
