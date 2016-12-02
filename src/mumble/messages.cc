/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    MRPB++ - An extensible Mumble bot
    Copyright (c) 2012 Matthew Perry (mattvperry)
    Copyright (c) 2013 Jeromy Maligie (Kingles)
    Copyright (c) 2013 Matthew Perry (mattvperry)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 niko20010
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
#include "mumble/messages.hh"

namespace Mumble
{
  std::unordered_map<std::type_index, uint16_t> Messages::sym_to_type
  {
    {std::type_index(typeid (MumbleProto::Version)), 0},
    {std::type_index(typeid (MumbleProto::UDPTunnel)), 1},
    {std::type_index(typeid (MumbleProto::Authenticate)), 2},
    {std::type_index(typeid (MumbleProto::Ping)), 3},
    {std::type_index(typeid (MumbleProto::Reject)), 4},
    {std::type_index(typeid (MumbleProto::ServerSync)), 5},
    {std::type_index(typeid (MumbleProto::ChannelRemove)), 6},
    {std::type_index(typeid (MumbleProto::ChannelState)), 7},
    {std::type_index(typeid (MumbleProto::UserRemove)), 8},
    {std::type_index(typeid (MumbleProto::UserState)), 9},
    {std::type_index(typeid (MumbleProto::BanList)), 10},
    {std::type_index(typeid (MumbleProto::TextMessage)), 11},
    {std::type_index(typeid (MumbleProto::PermissionDenied)), 12},
    {std::type_index(typeid (MumbleProto::ACL)), 13},
    {std::type_index(typeid (MumbleProto::QueryUsers)), 14},
    {std::type_index(typeid (MumbleProto::CryptSetup)), 15},
    {std::type_index(typeid (MumbleProto::ContextActionModify)), 16},
    {std::type_index(typeid (MumbleProto::ContextAction)), 17},
    {std::type_index(typeid (MumbleProto::UserList)), 18},
    {std::type_index(typeid (MumbleProto::VoiceTarget)), 19},
    {std::type_index(typeid (MumbleProto::PermissionQuery)), 20},
    {std::type_index(typeid (MumbleProto::CodecVersion)), 21},
    {std::type_index(typeid (MumbleProto::UserStats)), 22},
    {std::type_index(typeid (MumbleProto::RequestBlob)), 23},
    {std::type_index(typeid (MumbleProto::ServerConfig)), 24},
    {std::type_index(typeid (MumbleProto::SuggestConfig)), 25}
  };

  std::unique_ptr<::google::protobuf::Message> Messages::msg_from_type (
    uint16_t type)
  {
    switch (type)
      {
      case 0:
        return std::make_unique<MumbleProto::Version> ();
      case 1:
        return std::make_unique<MumbleProto::UDPTunnel> ();
      case 2:
        return std::make_unique<MumbleProto::Authenticate> ();
      case 3:
        return std::make_unique<MumbleProto::Ping> ();
      case 4:
        return std::make_unique<MumbleProto::Reject> ();
      case 5:
        return std::make_unique<MumbleProto::ServerSync> ();
      case 6:
        return std::make_unique<MumbleProto::ChannelRemove> ();
      case 7:
        return std::make_unique<MumbleProto::ChannelState> ();
      case 8:
        return std::make_unique<MumbleProto::UserRemove> ();
      case 9:
        return std::make_unique<MumbleProto::UserState> ();
      case 10:
        return std::make_unique<MumbleProto::BanList> ();
      case 11:
        return std::make_unique<MumbleProto::TextMessage> ();
      case 12:
        return std::make_unique<MumbleProto::PermissionDenied> ();
      case 13:
        return std::make_unique<MumbleProto::ACL> ();
      case 14:
        return std::make_unique<MumbleProto::QueryUsers> ();
      case 15:
        return std::make_unique<MumbleProto::CryptSetup> ();
      case 16:
        return std::make_unique<MumbleProto::ContextActionModify> ();
      case 17:
        return std::make_unique<MumbleProto::ContextAction> ();
      case 18:
        return std::make_unique<MumbleProto::UserList> ();
      case 19:
        return std::make_unique<MumbleProto::VoiceTarget> ();
      case 20:
        return std::make_unique<MumbleProto::PermissionQuery> ();
      case 21:
        return std::make_unique<MumbleProto::CodecVersion> ();
      case 22:
        return std::make_unique<MumbleProto::UserStats> ();
      case 23:
        return std::make_unique<MumbleProto::RequestBlob> ();
      case 24:
        return std::make_unique<MumbleProto::ServerConfig> ();
      case 25:
        return std::make_unique<MumbleProto::SuggestConfig> ();
      default:
        throw std::string ("Invalid message type");
      }
  }

}

