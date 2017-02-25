/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 dafoxia
    Copyright (c) 2014 niko20010
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 loscoala
    Copyright (c) 2016 dafoxia
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

#include <cstddef>
#include <chrono>
#include <memory>

#include "aither/log.hh"
#include "filesystem/filesystem.hh"
#include "mumble/codec.hh"
#include "mumble/connection.hh"

namespace Mumble
{
  class AudioPlayer
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    AudioPlayer (const Aither::Log &log, Codec codec, Connection &connection,
                 size_t sample_rate, size_t bitrate);
    ~AudioPlayer();
    void codec (Codec val);
    void volume (int vol);
    int volume () const;
    bool playing () const;
    // TODO: Split into three separate classes
    // (i.e. FileAudioPlayer, PipeAudioPlayer and PortaudioPlayer)
    void play_file (const FileSystem::path &file);
    void stream_named_pipe (const FileSystem::path &pipe);
    bool stream_portaudio ();
    void stop ();
    void bitrate (size_t bitrate);
    int bitrate () const;
    void framelength (const std::chrono::milliseconds ms);
    std::chrono::milliseconds framelength ();
  };
}
