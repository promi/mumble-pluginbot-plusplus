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
#include "mpd/audio-format.hh"

namespace Mpd
{
  struct AudioFormat::Impl
  {
    mpd_audio_format m_audio_format;
    Impl (const mpd_audio_format &audio_format) : m_audio_format (audio_format)
    {
    }
  };

  AudioFormat::AudioFormat (const mpd_audio_format &audio_format)
    : pimpl (std::make_unique<Impl> (audio_format))
  {
  }

  AudioFormat::AudioFormat (const AudioFormat &other)
    : AudioFormat (other.pimpl->m_audio_format)
  {
  }

  AudioFormat::~AudioFormat ()
  {
  }

  uint32_t AudioFormat::sample_rate () const
  {
    return pimpl->m_audio_format.sample_rate;
  }

  uint8_t AudioFormat::bits () const
  {
    return pimpl->m_audio_format.bits;
  }

  uint8_t AudioFormat::channels () const
  {
    return pimpl->m_audio_format.channels;
  }
}
