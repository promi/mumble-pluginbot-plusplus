/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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

#include <mpd/client.h>

namespace Mpd
{
  enum class Error : uint8_t
  {
    // no error
    Success = MPD_ERROR_SUCCESS,
    // out of memory
    OutOfMemory = MPD_ERROR_OOM,
    // a function was called with an unrecognized or invalid argument
    InvalidArgument = MPD_ERROR_ARGUMENT,
    // a function was called which is not available in the current state of libmpdclient
    InvalidState = MPD_ERROR_STATE,
    // timeout trying to talk to mpd
    Timeout = MPD_ERROR_TIMEOUT,
    // system error
    SystemError = MPD_ERROR_SYSTEM,
    // unknown host
    UnknownHost = MPD_ERROR_RESOLVER,
    // malformed response received from MPD
    MalformedResponse = MPD_ERROR_MALFORMED,
    // connection closed by mpd
    ConnectionClosed = MPD_ERROR_CLOSED,
    // The server has returned an error code,
    // which can be queried with mpd_connection_get_server_error().
    ServerError = MPD_ERROR_SERVER
  };
}
