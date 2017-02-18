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

#include <string>
#include <mpd/client.h>

namespace Mpd
{
  enum class TagType
  {
    // Special value returned by mpd_tag_name_parse() when an unknown name was passed.
    Unknown = MPD_TAG_UNKNOWN,
    Artist = MPD_TAG_ARTIST,
    // ArtistSort = MPD_TAG_ARTIST_SORT,
    Album = MPD_TAG_ALBUM,
    // AlbumSort = MPD_TAG_ALBUM_SORT,
    AlbumArtist = MPD_TAG_ALBUM_ARTIST,
    // AlbumArtistSort = MPD_TAG_ALBUM_ARTIST_SORT,
    Title = MPD_TAG_TITLE,
    Track = MPD_TAG_TRACK,
    Name = MPD_TAG_NAME,
    Genre = MPD_TAG_GENRE,
    Date = MPD_TAG_DATE,
    Composer = MPD_TAG_COMPOSER,
    Performer = MPD_TAG_PERFORMER,
    Comment = MPD_TAG_COMMENT,
    Disc = MPD_TAG_DISC,
    MusicbrainzArtistID = MPD_TAG_MUSICBRAINZ_ARTISTID,
    MusicbrainzAlbumID = MPD_TAG_MUSICBRAINZ_ALBUMID,
    MusicbrainzAlbumArtistID = MPD_TAG_MUSICBRAINZ_ALBUMARTISTID,
    MusicbrainzTrackID = MPD_TAG_MUSICBRAINZ_TRACKID,
#ifdef MPD_TAG_MUSICBRAINZ_RELEASETRACKID
    MusicbrainzReleaseTrackID = MPD_TAG_MUSICBRAINZ_RELEASETRACKID,
#endif
  };

  /**
  * Looks up the name of the specified tag.
  *
  * @return the name, or NULL if the tag type is not valid
  */
  std::string *
  tag_name(TagType type);

  /**
   * Parses a tag name, and returns its #mpd_tag_type value.
   *
   * @return a #mpd_tag_type value, or MPD_TAG_UNKNOWN if the name was
   * not recognized
   */
  TagType
  tag_name_parse(const std::string &name);

  /**
   * Same as mpd_tag_name_parse(), but ignores case.
   *
   * @return a #mpd_tag_type value, or MPD_TAG_UNKNOWN if the name was
   * not recognized
   */
  TagType
  tag_name_iparse(const std::string &name);

}
