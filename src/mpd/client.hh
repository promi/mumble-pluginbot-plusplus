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

#include <memory>
#include <string>
#include <cstdint>
#include <vector>

#include "mpd/error.hh"
#include "mpd/idle.hh"
#include "mpd/flag-set.hh"
#include "mpd/status.hh"
#include "mpd/song.hh"
#include "mpd/operator.hh"
#include "mpd/playlist.hh"

namespace Mpd
{
  class Client
  {
  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
  public:
    Client (const std::string& host = "", uint16_t port = 0,
            uint timeout_ms = 0);
    ~Client ();
    // Connection
    /*
      mpd_malloc struct mpd_connection*	mpd_connection_new_async (
      struct mpd_async *async, const char *welcome)
      const struct mpd_settings* mpd_connection_get_settings (
      const struct mpd_connection *connection)
      void 	mpd_connection_set_keepalive (
      struct mpd_connection *connection, bool keepalive)
      void 	mpd_connection_set_timeout (
      struct mpd_connection *connection, unsigned timeout_ms)
      mpd_pure int 	mpd_connection_get_fd (const struct mpd_connection *connection)
      mpd_pure struct mpd_async * 	mpd_connection_get_async (
      struct mpd_connection *connection)
    */
    Error	error ();
    std::string	error_message ();
    /*
      mpd_pure enum mpd_server_error 	mpd_connection_get_server_error (
      const struct mpd_connection *connection)
      mpd_pure unsigned 	mpd_connection_get_server_error_location (
      const struct mpd_connection *connection)
      mpd_pure int 	mpd_connection_get_system_error (
      const struct mpd_connection *connection)
    */
    void clear_error ();
    /*
      mpd_pure const unsigned * 	mpd_connection_get_server_version (const struct mpd_connection *connection)
      mpd_pure int 	mpd_connection_cmp_server_version (const struct mpd_connection *connection, unsigned major, unsigned minor, unsigned patch)
    */
    // Idle
    static std::string idle_name (Idle idle);
    static Idle idle_name_parse (const std::string &name);
    void send_idle ();
    void send_idle_mask (FlagSet<Idle> mask);
    void send_noidle ();
    static Idle idle_parse_pair (std::pair<std::string, std::string> &pair);
    FlagSet<Idle> recv_idle (bool disable_timeout);
    FlagSet<Idle> idle ();
    FlagSet<Idle> idle_mask (FlagSet<Idle> mask);
    FlagSet<Idle> noidle ();
    // Status
    void send_status ();
    Status recv_status ();
    Status status ();
    // Song
    std::unique_ptr<Song> recv_song ();
    std::vector<std::unique_ptr<Song>> recv_songs ();
    // Player
    void send_current_song ();
    std::unique_ptr<Song> current_song ();
    void send_play ();
    void play ();
    void send_play_pos (unsigned song_pos);
    void play_pos (unsigned song_pos);
    void send_play_id (unsigned id);
    void play_id (unsigned song_id);
    void send_stop ();
    void stop ();
    void send_toggle_pause ();
    void toggle_pause ();
    void send_pause (bool mode);
    void pause (bool mode);
    void send_next ();
    void next ();
    void send_previous ();
    void previous ();
    void send_seek_pos (unsigned song_pos, unsigned t);
    void seek_pos (unsigned song_pos, unsigned t);
    void send_seek_id (unsigned id, unsigned t);
    void seek_id (unsigned song_id, unsigned t);
    void send_repeat (bool mode);
    void repeat (bool mode);
    void send_random (bool mode);
    void random (bool mode);
    void send_single (bool mode);
    void single (bool mode);
    void send_consume (bool mode);
    void consume (bool mode);
    void send_crossfade (unsigned seconds);
    void crossfade (unsigned seconds);
    void send_mixrampdb (float db);
    void mixrampdb (float db);
    void send_mixrampdelay (float seconds);
    void mixrampdelay (float seconds);
    void send_clearerror ();
    void clearerror ();
    // Mixer
    void send_volume (unsigned volume);
    void volume (unsigned volume);
    void send_change_volume (int relative_volume);
    void change_volume (int relative_volume);
    // Queue
    void send_list_queue_meta ();
    void send_list_queue_range_meta (unsigned start, unsigned end);
    void send_get_queue_song_pos (unsigned pos);
    Song get_queue_song_pos (unsigned pos);
    void send_get_queue_song_id (unsigned id);
    Song get_queue_song_id (unsigned id);
    void send_queue_changes_meta (unsigned version);
    void send_queue_changes_brief (unsigned version);
    void recv_queue_change_brief (unsigned *position_r, unsigned *id_r);
    void send_add (const std::string &file);
    void add (const std::string &uri);
    void send_add_id (const std::string &file);
    void send_add_id_to (const std::string &uri, unsigned to);
    int recv_song_id ();
    int add_id (const std::string &file);
    int add_id_to (const std::string &uri, unsigned to);
    void send_delete (unsigned pos);
    void delete_ (unsigned pos);
    void send_delete_range (unsigned start, unsigned end);
    void delete_range (unsigned start, unsigned end);
    void send_delete_id (unsigned id);
    void delete_id (unsigned id);
    void send_shuffle ();
    void shuffle ();
    void send_shuffle_range (unsigned start, unsigned end);
    void shuffle_range (unsigned start, unsigned end);
    void send_clear ();
    void clear ();
    void send_move (unsigned from, unsigned to);
    void move (unsigned from, unsigned to);
    void send_move_id (unsigned from, unsigned to);
    void move_id (unsigned from, unsigned to);
    void send_move_range (unsigned start, unsigned end, unsigned to);
    void move_range (unsigned start, unsigned end, unsigned to);
    void send_swap (unsigned pos1, unsigned pos2);
    void swap (unsigned pos1, unsigned pos2);
    void send_swap_id (unsigned id1, unsigned id2);
    void swap_id (unsigned id1, unsigned id2);
    void send_prio (int priority, unsigned position);
    void prio (int priority, unsigned position);
    void send_prio_range (int priority, unsigned start, unsigned end);
    void prio_range (int priority, unsigned start, unsigned end);
    void send_prio_id (int priority, unsigned id);
    void prio_id (int priority, unsigned id);
    // Search
    void search_db_songs (bool exact);
    void search_add_db_songs (bool exact);
    void search_queue_songs (bool exact);
    void search_db_tags (TagType type);
    void count_db_songs ();
    void search_add_base_constraint (Operator oper, const std::string &value);
    void search_add_uri_constraint (Operator oper, const std::string &value);
    void search_add_tag_constraint (Operator oper, TagType type,
                                    const std::string &value);
    void search_add_any_tag_constraint (Operator oper, const std::string &value);
    void search_add_modified_since_constraint (Operator oper, time_t value);
    void search_add_window (unsigned start, unsigned end);
    void search_commit ();
    void search_cancel ();
    std::pair<std::string, std::string> recv_pair_tag (TagType type);
    // Database
    void send_list_all (const std::string &path);
    void send_list_all_meta (const std::string &path);
    void send_list_meta (const std::string &path);
    void send_read_comments (const std::string &path);
    void send_update (const std::string &path);
    void send_rescan (const std::string &path);
    unsigned recv_update_id ();
    unsigned update (const std::string &path);
    unsigned rescan (const std::string &path);
    // Playlist
    void send_list_playlists ();
    std::unique_ptr<Playlist> recv_playlist ();
    std::vector<std::unique_ptr<Playlist>> recv_playlists ();
    void send_list_playlist (const std::string &name);
    void send_list_playlist_meta (const std::string &name);
    void send_playlist_clear (const std::string &name);
    void playlist_clear (const std::string &name);
    void send_playlist_add (const std::string &name, const std::string &path);
    void playlist_add (const std::string &name, const std::string &path);
    void send_playlist_move (const std::string &name, unsigned from, unsigned to);
    void send_playlist_delete (const std::string &name, unsigned pos);
    void playlist_delete (const std::string &name, unsigned pos);
    void send_save (const std::string &name);
    void save (const std::string &name);
    void send_load (const std::string &name);
    void load (const std::string &name);
    void send_rename (const std::string &from, const std::string &to);
    void rename (const std::string &from, const std::string &to);
    void send_rm (const std::string &name);
    void rm (const std::string &name);
  };
}
