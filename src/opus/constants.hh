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

#include <opus/opus.h>

namespace Opus
{
  enum class SampleRate : opus_int32
  {
    fs_8k = 8'000,
            fs_12k = 12'000,
    fs_16k = 16'000,
             fs_24k = 24'000,
    fs_48k = 48'000
             /*'*/
  };

  enum class Channels : int
  {
    mono = 1,
    stereo = 2
  };

  enum class Application : opus_int32
  {
    voip = OPUS_APPLICATION_VOIP,
    audio = OPUS_APPLICATION_AUDIO,
    restricted_lowdelay = OPUS_APPLICATION_RESTRICTED_LOWDELAY
  };

  enum class Signal : opus_int32
  {
    auto_ = OPUS_AUTO,
    voice = OPUS_SIGNAL_VOICE,
    music = OPUS_SIGNAL_MUSIC
  };
}
