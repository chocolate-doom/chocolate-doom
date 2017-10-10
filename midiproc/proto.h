//
// Copyright(C) 2017 Alex Mayfield
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Headers for all types of midipipe messages.
//

#ifndef __PROTO__
#define __PROTO__

typedef enum {
    MIDIPIPE_PACKET_TYPE_REGISTER_SONG,
    MIDIPIPE_PACKET_TYPE_REGISTER_SONG_ACK,
    MIDIPIPE_PACKET_TYPE_SET_VOLUME,
    MIDIPIPE_PACKET_TYPE_PLAY_SONG,
    MIDIPIPE_PACKET_TYPE_STOP_SONG,
    MIDIPIPE_PACKET_TYPE_SHUTDOWN
} net_midipipe_packet_type_t;

#endif

