/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2012 Matthew Perry (mattvperry)
    Copyright (c) 2013 Jeromy Maligie (Kingles)
    Copyright (c) 2013 Matthew Perry (mattvperry)
    Copyright (c) 2014 Aaron Herting (qwertos)
    Copyright (c) 2014 Benjamin Neff (SuperTux88)
    Copyright (c) 2014 Jack Chen (chendo)
    Copyright (c) 2014 Matthew Perry (mattvperry)
    Copyright (c) 2014 Meister der Bots
    Copyright (c) 2014 dafoxia
    Copyright (c) 2015 dafoxia
    Copyright (c) 2015 loscoala
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
#include "mumble/udp-packet.hh"

namespace Mumble
{
  std::vector<uint8_t> UDPPacket::data (PacketDataStream &pds)
  {
    pds.rewind ();
    uint8_t header = (static_cast<uint8_t> (this->type) << 5) |
                     (this->target & 0x1F);
    pds.append (header);
    if (this->type == Codec::ping)
      {
        pds.put_int (this->timestamp);
      }
    else
      {
        pds.put_int (this->sequence_number);
        if (this->type == Codec::opus)
          {
            // TODO: This only works for OPUS. CELT and SPEEX use a multi frame model
            // where the frames have to be packed / unpacked individually
            pds.append (this->payload.size ());
            pds.append_block (this->payload);
          }
        else
          {
            throw std::string ("packet type not yet supported");
          }
        // TODO: Add position_info data
      }
    auto packet_size = pds.size ();
    pds.rewind ();
    return pds.get_block (packet_size);
  }

  void UDPPacket::data (PacketDataStream &pds, const std::vector<uint8_t> &data)
  {
    pds.rewind ();
    pds.append_block (data);
    pds.rewind ();

    uint8_t header = pds.get_next ();
    this->type = static_cast<Codec> (header >> 5);
    this->target = header & 0x1F;

    if (this->type == Codec::ping)
      {
        this->timestamp = pds.get_int ();
      }
    else
      {
        this->source_session_id = pds.get_int ();
        this->sequence_number = pds.get_int ();
        if (this->type == Codec::opus)
          {
            auto len = pds.get_int ();
            this->payload = pds.get_block (len);
          }
        else
          {
            throw std::string ("packet type not yet supported");
            /*
                case Codec::alpha:

                  auto len = m_pds.get_next ();
                  auto audio = m_pds.get_block (len & 0x7f);
                  queue << celt_decoder.decode (audio.join());
                  while ((len & 0x80) != 0)
                          {
                  len = m_pds.get_next ();
                  audio = m_pds.get_block (len & 0x7f);
                            if (len & 0x7f != 0)
                  queue << celt_decoder.decode (audio.join ());
                          }

                  break;
                case Codec::beta:

                  auto len = m_pds.get_next ();
                  auto audio = m_pds.get_block (len & 0x7f);
                  queue << m_celt_decoder.decode (audio.join ());
                  while ((len & 0x80) != 0)
                          {
                  len = m_pds.get_next ();
                  audio += m_pds.get_block (len & 0x7f);
                  queue << celt_decoder.decode (audio.join());
                  end
            */
          }
        // TODO: Read position info data (if available)
      }
  }
}
