/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
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

#include <mpd/client.h>

namespace Mpd
{
  enum class Idle : uint16_t
  {
    // song database has been updated
    Database = MPD_IDLE_DATABASE,
    // a stored playlist has been modified, created, deleted or renamed
    StoredPlaylist = MPD_IDLE_STORED_PLAYLIST,
    // the queue has been modified
    Queue = MPD_IDLE_QUEUE,
    // the player state has changed: play, stop, pause, seek, ...
    Player = MPD_IDLE_PLAYER,
    // the volume has been modified
    Mixer = MPD_IDLE_MIXER,
    // an audio output device has been enabled or disabled
    Output = MPD_IDLE_OUTPUT,
    // options have changed: crossfade, random, repeat, ...
    Options = MPD_IDLE_OPTIONS,
    // a database update has started or finished.
    Update = MPD_IDLE_UPDATE,
    // a sticker has been modified.
    Sticker = MPD_IDLE_STICKER,
    // a client has subscribed to or unsubscribed from a channel
    Subscription = MPD_IDLE_SUBSCRIPTION,
    // a message on a subscribed channel was received
    Message = MPD_IDLE_MESSAGE
  };
}
