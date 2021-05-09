//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	System interface for music.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "i_glob.h"
#include "i_midipipe.h"

#include "config.h"
#include "doomtype.h"
#include "memio.h"
#include "mus2mid.h"

#include "deh_str.h"
#include "gusconf.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_swap.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "sha1.h"
#include "w_wad.h"
#include "z_zone.h"

#define MID_HEADER_MAGIC "MThd"
#define MUS_HEADER_MAGIC "MUS\x1a"

#define FLAC_HEADER "fLaC"
#define OGG_HEADER "OggS"

// Looping Vorbis metadata tag names. These have been defined by ZDoom
// for specifying the start and end positions for looping music tracks
// in .ogg and .flac files.
// More information is here: http://zdoom.org/wiki/Audio_loop
#define LOOP_START_TAG "LOOP_START"
#define LOOP_END_TAG   "LOOP_END"

// FLAC metadata headers that we care about.
#define FLAC_STREAMINFO      0
#define FLAC_VORBIS_COMMENT  4

// Ogg metadata headers that we care about.
#define OGG_ID_HEADER        1
#define OGG_COMMENT_HEADER   3

// Structure for music substitution.
// We store a mapping based on SHA1 checksum -> filename of substitute music
// file to play, so that substitution occurs based on content rather than
// lump name. This has some inherent advantages:
//  * Music for Plutonia (reused from Doom 1) works automatically.
//  * If a PWAD replaces music, the replacement music is used rather than
//    the substitute music for the IWAD.
//  * If a PWAD reuses music from an IWAD (even from a different game), we get
//    the high quality version of the music automatically (neat!)

typedef struct
{
    const char *hash_prefix;
    const char *filename;
} subst_music_t;

// Structure containing parsed metadata read from a digital music track:
typedef struct
{
    boolean valid;
    unsigned int samplerate_hz;
    int start_time, end_time;
} file_metadata_t;

static subst_music_t *subst_music = NULL;
static unsigned int subst_music_len = 0;

static boolean music_initialized = false;

// If this is true, this module initialized SDL sound and has the 
// responsibility to shut it down

static boolean sdl_was_initialized = false;

char *music_pack_path = "";

// If true, we are playing a substitute digital track rather than in-WAD
// MIDI/MUS track, and file_metadata contains loop metadata.
static file_metadata_t file_metadata;

// Position (in samples) that we have reached in the current track.
// This is updated by the TrackPositionCallback function.
static unsigned int current_track_pos;

// Currently playing music track.
static Mix_Music *current_track_music = NULL;

// If true, the currently playing track is being played on loop.
static boolean current_track_loop;

// Table of known hashes and filenames to look up for them. This allows
// users to drop in a set of files without having to also provide a
// configuration file.
static const subst_music_t known_filenames[] = {
    // Doom 1 music files.
    {"b2e05b4e8dff8d76f8f4", "d_inter.{ext}"},
    {"0c0acce45130bab935d2", "d_intro.{ext}"},
    {"fca4086939a68ae4ed84", "d_victor.{ext}"},
    {"5971e5e20554f47ca065", "d_intro.{ext}"},
    {"99767e32769229897f77", "d_e1m1.{ext}"},
    {"b5e7dfb4efe9e688bf2a", "d_e1m2.{ext}"},
    {"fda8fa73e4d30a6b961c", "d_e1m3.{ext}"},
    {"3805f9bf3f1702f7e7f5", "d_e1m4.{ext}"},
    {"f546ed823b234fe39165", "d_e1m5.{ext}"},
    {"4450811b5a6748cfd83e", "d_e1m6.{ext}"},
    {"73edb50d96b0ac03be34", "d_e1m7.{ext}"},
    {"47d711a6fd32f5047879", "d_e1m8.{ext}"},
    {"62c631c2fdaa5ecd9a8d", "d_e1m9.{ext}"},
    {"7702a6449585428e7185", "d_e2m1.{ext}"},
    {"1cb1810989cbfae2b29b", "d_e2m2.{ext}"},
    {"7d740f3c881a22945e47", "d_e2m4.{ext}"},
    {"ae9c3dc2f9aeea002327", "d_e2m6.{ext}"},
    {"b26aad3caa420e9a2c76", "d_e2m7.{ext}"},
    {"90f06251a2a90bfaefd4", "d_e2m8.{ext}"},
    {"b2fb439f23c08c8e2577", "d_e3m1.{ext}"},
    {"b6c07bb249526b864208", "d_e3m2.{ext}"},
    {"ce3587ee503ffe707b2d", "d_e3m3.{ext}"},
    {"d746ea2aa16b3237422c", "d_e3m8.{ext}"},
    {"3da3b1335560a92912e6", "d_bunny.{ext}"},

    // Duplicates that don't have identical hashes:
    {"4a5badc4f10a7d4ed021", "d_inter.{ext}"},  // E2M3
    {"36b14bf165b3fdd3958e", "d_e1m7.{ext}"},   // E3M5
    {"e77c3d42f2ea87f04607", "d_e1m6.{ext}"},   // E3M6
    {"3d85ec9c10b5ea465568", "d_e2m7.{ext}"},   // E3M7
    {"4d42e2ce1c1ff192500e", "d_e1m9.{ext}"},   // E3M9

    // These tracks are reused in Alien Vendetta, but are MIDs:
    {"a05e45f67e1b64733fe3", "d_e2m1.{ext}"},   // MAP02
    {"8024ae1616ddd97ce330", "d_e1m4.{ext}"},   // MAP03
    {"3af8d79ddba49edaf9eb", "d_victor.{ext}"}, // MAP05
    {"a55352c96c025b6bd08a", "d_inter.{ext}"},  // MAP07
    {"76d1fc25ab7b1b4a58d6", "d_e1m8.{ext}"},   // MAP11
    {"497777f0863eca7cea87", "d_e1m2.{ext}"},   // MAP12
    {"0228fd87f8762f112fb6", "d_e2m2.{ext}"},   // MAP13
    {"db94e8e1d7c02092eab5", "d_e1m6.{ext}"},   // MAP14
    {"5a8d7a307eebc952795c", "d_e2m7.{ext}"},   // MAP16
    {"1a36b692bf26d94a72cc", "d_e1m7.{ext}"},   // MAP23
    {"37c6cefa351b06995152", "d_e1m5.{ext}"},   // MAP27
    {"36b97b87fe98348d44b6", "d_e2m6.{ext}"},   // MAP28

    // Doom II music files.
    {"79080e9681a2d7bec3fb", "d_runnin.{ext}"},  // MAP01,15
    {"868b3aae73c7b12e92c0", "d_stalks.{ext}"},  // MAP02,11,17
    {"19237754d2eb85f41d84", "d_countd.{ext}"},  // MAP03,21
    {"00abff3b61b25a6855d2", "d_betwee.{ext}"},  // MAP04
    {"954636c7ee09edf5d98f", "d_doom.{ext}"},    // MAP05,13
    {"8d32b2b7aa3b806474c1", "d_the_da.{ext}"},  // MAP06,12,24
    {"41efc3c84bb321af2b6b", "d_shawn.{ext}"},   // MAP07,19,29
    // Assuming single D_DDTBLU: http://doomwiki.org/wiki/Doom_II_music#Trivia
    {"51c0872fec9f43259318", "d_ddtblu.{ext}"},  // MAP08
    {"acb7ad85494d18235df8", "d_ddtblu.{ext}"},  // MAP14,22
    {"4b7ceccbf47e78e2fa0b", "d_in_cit.{ext}"},  // MAP09
    {"1d1f4a9edba174584e11", "d_dead.{ext}"},    // MAP10,16
    {"1736c81aac77f9bffd3d", "d_romero.{ext}"},  // MAP18,27
    {"a55d400570ad255a576b", "d_messag.{ext}"},  // MAP20,26
    {"29d30c3fbd712016f2e5", "d_ampie.{ext}"},   // MAP23
    {"bcfe9786afdcfb704afa", "d_adrian.{ext}"},  // MAP25
    {"e05c10389e71836834ae", "d_tense.{ext}"},   // MAP28
    {"b779022b1d0f0010b8f0", "d_openin.{ext}"},  // MAP30
    {"a9a5f7b0ab3be0f4fc24", "d_evil.{ext}"},    // MAP31
    {"4503d155aafec0296689", "d_ultima.{ext}"},  // MAP32
    {"56f2363f01df38908c77", "d_dm2ttl.{ext}"},
    {"71e58baf9e9dea4dd24a", "d_dm2int.{ext}"},
    {"e632318629869811f7dc", "d_read_m.{ext}"},

    // Duplicate filenames: the above filenames are the "canonical" files
    // for the given SHA1 hashes, but we can also look for these filenames
    // corresponding to the duplicated music tracks too.
    {"868b3aae73c7b12e92c0", "d_stlks2.{ext}"},
    {"868b3aae73c7b12e92c0", "d_stlks3.{ext}"},
    {"8d32b2b7aa3b806474c1", "d_theda2.{ext}"},
    {"8d32b2b7aa3b806474c1", "d_theda3.{ext}"},
    {"954636c7ee09edf5d98f", "d_doom2.{ext}"},
    {"acb7ad85494d18235df8", "d_ddtbl2.{ext}"},
    {"acb7ad85494d18235df8", "d_ddtbl3.{ext}"},
    {"79080e9681a2d7bec3fb", "d_runni2.{ext}"},
    {"1d1f4a9edba174584e11", "d_dead2.{ext}"},
    {"41efc3c84bb321af2b6b", "d_shawn2.{ext}"},
    {"41efc3c84bb321af2b6b", "d_shawn3.{ext}"},
    {"19237754d2eb85f41d84", "d_count2.{ext}"},
    {"a55d400570ad255a576b", "d_messg2.{ext}"},
    {"1736c81aac77f9bffd3d", "d_romer2.{ext}"},

    // These tracks are reused in Alien Vendetta, but are MIDs:
    {"9433604c098b7b1119a4", "d_in_cit.{ext}"},  // MAP26

    // Heretic tracks.
    {"12818ca0d3c957e7d57e", "mus_titl.{ext}"},
    {"5cb988538ce1b1857349", "mus_intr.{ext}"},
    {"6f126abe35a78b61b930", "mus_cptd.{ext}"},
    {"62557250f0427c067dc9", "mus_e1m1.{ext}"},
    {"1e8d5fd814490b9ae166", "mus_e1m2.{ext}"},
    {"f0f31e8834e85035d434", "mus_e1m3.{ext}"},
    {"054d6997405cc5a32b46", "mus_e1m4.{ext}"},
    {"31950ab062cc1e5ca49d", "mus_e1m5.{ext}"},
    {"7389024fbab0dff47211", "mus_e1m6.{ext}"},
    {"f2aa312dddd0a294a095", "mus_e1m7.{ext}"},
    {"cd6856731d1ae1f3aa4e", "mus_e1m8.{ext}"},
    {"d7fe793f266733d92e61", "mus_e1m9.{ext}"},
    {"933545b48fad8c66f042", "mus_e2m1.{ext}"},
    {"bf88ecd4ae1621222592", "mus_e2m2.{ext}"},
    {"4f619f87a828c2ca4801", "mus_e2m3.{ext}"},
    {"13033a83c49424b2f2ab", "mus_e2m4.{ext}"},
    {"b3851f9351ae411d9de3", "mus_e2m6.{ext}"},
    {"82539791159fbbc02a23", "mus_e2m7.{ext}"},
    {"fd9e53a49cfa62c463a0", "mus_e2m8.{ext}"},
    {"29503959324d2ca67958", "mus_e2m9.{ext}"},
    {"3aa632257c5be375b97b", "mus_e3m2.{ext}"},
    {"69ba0dce7913d53b67a8", "mus_e3m3.{ext}"},

    // These Heretic tracks are reused in Alien Vendetta, but are MIDs:
    {"51344131e8d260753ce7", "mus_e2m3.{ext}"},  // MAP15
    {"78b570b2397570440aff", "mus_e1m1.{ext}"},  // MAP19
    {"ee21ba9fad4de3dfaef0", "mus_e1m4.{ext}"},  // MAP29
    {"d2bb643a60696ccbca03", "mus_e1m9.{ext}"},  // MAP32

    // Hexen tracks:
    {"fbf55fc1ee26bd01266b", "winnowr.{ext}"},
    {"71776e2da2b7ba607d81", "jachr.{ext}"},
    {"c5c8630608b8132b33cd", "simonr.{ext}"},
    {"43683b3f55a031de88d4", "wutzitr.{ext}"},
    {"a6062883f29436ef73db", "falconr.{ext}"},
    {"512cb6cc9b558d5f0fef", "levelr.{ext}"},
    {"d31226ae75fce6a24208", "chartr.{ext}"},
    {"bf1f1e561bbdba4e699f", "swampr.{ext}"},
    {"b303193f756ca0e2de0f", "deepr.{ext}"},
    {"f0635f0386d883b00186", "fubasr.{ext}"},
    {"18f2a01f83df6e3abedc", "grover.{ext}"},
    {"b2527eb0522f08b2cf5f", "fortr.{ext}"},
    {"343addba8ba53a20a160", "foojar.{ext}"},
    {"c13109045b06b5a63386", "sixater.{ext}"},
    {"693525aaf69eac5429ab", "wobabyr.{ext}"},
    {"8f884223811c2bb8311d", "cryptr.{ext}"},
    {"de540e6826e62b32c01c", "fantar.{ext}"},
    {"efdff548df918934f71f", "blechr.{ext}"},
    {"de91f150f6a127e72e35", "voidr.{ext}"},
    {"e0497fe27289fe18515b", "chap_1r.{ext}"},
    {"f2ef1abdc3f672a3519a", "chap_2r.{ext}"},
    {"78cd9882f61cc441bef4", "chap_3r.{ext}"},
    {"97b2b575d9d096c1f89f", "chap_4r.{ext}"},
    {"ad0197a0f6c52ac30915", "chippyr.{ext}"},
    {"30506c62e9f0989ffe09", "percr.{ext}"},
    {"3542803beaa43bf1de1a", "secretr.{ext}"},
    {"81067721f40c611d09fb", "bonesr.{ext}"},
    {"4822af2e1a2eb7faf660", "octor.{ext}"},
    {"26bb3cec902ed8008fc2", "rithmr.{ext}"},
    {"94ab641c7aa93caac77a", "stalkr.{ext}"},
    {"d0a3f337c54b0703b4d3", "borkr.{ext}"},
    {"79e7781ec7eb9b9434b5", "crucibr.{ext}"},
    {"c2786e5581a7f8801969", "hexen.{ext}"},
    {"97fae9a084c0efda5151", "hub.{ext}"},
    {"c5da52d5c2ec4803ef8f", "hall.{ext}"},
    {"1e71bc0e2feafb06214e", "orb.{ext}"},
    {"bc9dcfa6632e847e03af", "chess.{ext}"},

    // Hexen CD tracks: alternate filenames for a ripped copy of
    // the CD soundtrack.
    {"71776e2da2b7ba607d81", "hexen02.{ext}"},   // level  2 (jachr)
    {"efdff548df918934f71f", "hexen03.{ext}"},   // level 26 (blechr)
    {"c2786e5581a7f8801969", "hexen04.{ext}"},   // (hexen)
    {"1e71bc0e2feafb06214e", "hexen05.{ext}"},   // (orb)
    {"f0635f0386d883b00186", "hexen06.{ext}"},   // level 10 (fubasr)
    {"bc9dcfa6632e847e03af", "hexen07.{ext}"},   // (chess)
    {"8f884223811c2bb8311d", "hexen08.{ext}"},   // level 24 (cryptr)
    {"a6062883f29436ef73db", "hexen09.{ext}"},   // level  5 (falconr)
    {"4822af2e1a2eb7faf660", "hexen10.{ext}"},   // level 36 (octor)
    {"26bb3cec902ed8008fc2", "hexen11.{ext}"},   // level 37 (rithmr)
    {"c13109045b06b5a63386", "hexen12.{ext}"},   // level 22 (sixater)
    {"fbf55fc1ee26bd01266b", "hexen13.{ext}"},   // level  1 (winnowr)
    {"bf1f1e561bbdba4e699f", "hexen14.{ext}"},   // level  8 (swampr)
    {"43683b3f55a031de88d4", "hexen15.{ext}"},   // level  4 (wutzitr)
    {"81067721f40c611d09fb", "hexen16.{ext}"},   // level 35 (bonesr)
    {"e0497fe27289fe18515b", "hexen17.{ext}"},   // level 28 (chap_1r)
    {"97b2b575d9d096c1f89f", "hexen18.{ext}"},   // level 31 (chap_4r)
    {"de540e6826e62b32c01c", "hexen19.{ext}"},   // level 25 (fantar)
    {"343addba8ba53a20a160", "hexen20.{ext}"},   // level 21 (foojar)
    {"512cb6cc9b558d5f0fef", "hexen21.{ext}"},   // level  6 (levelr)
    {"c5c8630608b8132b33cd", "hexen22.{ext}"},   // level  3 (simonr)

    // Strife:
    {"8ac2b2b47707f0fdf8f6", "d_logo.{ext}"},   // Title
    {"62e1c58054a1f1bc39b2", "d_action.{ext}"}, // 1,15,28
    {"12fa000f3fa1edac5c4f", "d_tavern.{ext}"}, // 2
    {"695e56ab3251792d20e5", "d_danger.{ext}"}, // 3,11
    {"96fe30e8712217b60dd7", "d_fast.{ext}"},   // 4
    {"61345598a3de04aad508", "d_darker.{ext}"}, // 6,14
    {"52353e9a435b7b1cb268", "d_strike.{ext}"}, // 7,19
    {"061164504907bffc9c22", "d_slide.{ext}"},  // 8,18,22
    {"3dbb4b703ce69aafcdd5", "d_tribal.{ext}"}, // 9
    {"393773688eba050c3548", "d_march.{ext}"},  // 10
    {"3cba3c627de065a667dd", "d_mood.{ext}"},   // 12
    {"b1f65a333e5c70255784", "d_castle.{ext}"}, // 13
    {"e1455a83a04c9ac4a09f", "d_fight.{ext}"},  // 16,31
    {"17f822b7374b1f069b89", "d_spense.{ext}"}, // 17
    {"e66c5a1a7d05f021f4ae", "d_dark.{ext}"},   // 20
    {"1c92bd0625026af30dad", "d_tech.{ext}"},   // 21,27
    {"7ae280713d078de7933a", "d_drone.{ext}"},  // 23,30
    {"4a664afd0d7eae79c97a", "d_panthr.{ext}"}, // 24
    {"4a7d62beeac5601ccf21", "d_sad.{ext}"},    // 25
    {"e60e109779400f2855d7", "d_instry.{ext}"}, // 26,29
    {"b7d36878faeb291d6df5", "d_happy.{ext}"},  // Better ending
    {"ff4a342c8c5ec51b06c3", "d_end.{ext}"},    // Worse ending
    // This conflicts with Doom's d_intro:
    //{"ec8fa484c4e85adbf700", "d_intro.{ext}"},  // 5
};

// Given a time string (for LOOP_START/LOOP_END), parse it and return
// the time (in # samples since start of track) it represents.
static unsigned int ParseVorbisTime(unsigned int samplerate_hz, char *value)
{
    char *num_start, *p;
    unsigned int result = 0;
    char c;

    if (strchr(value, ':') == NULL)
    {
	return atoi(value);
    }

    result = 0;
    num_start = value;

    for (p = value; *p != '\0'; ++p)
    {
        if (*p == '.' || *p == ':')
        {
            c = *p; *p = '\0';
            result = result * 60 + atoi(num_start);
            num_start = p + 1;
            *p = c;
        }

        if (*p == '.')
        {
            return result * samplerate_hz
	         + (unsigned int) (atof(p) * samplerate_hz);
        }
    }

    return (result * 60 + atoi(num_start)) * samplerate_hz;
}

// Given a vorbis comment string (eg. "LOOP_START=12345"), set fields
// in the metadata structure as appropriate.
static void ParseVorbisComment(file_metadata_t *metadata, char *comment)
{
    char *eq, *key, *value;

    eq = strchr(comment, '=');

    if (eq == NULL)
    {
        return;
    }

    key = comment;
    *eq = '\0';
    value = eq + 1;

    if (!strcmp(key, LOOP_START_TAG))
    {
        metadata->start_time = ParseVorbisTime(metadata->samplerate_hz, value);
    }
    else if (!strcmp(key, LOOP_END_TAG))
    {
        metadata->end_time = ParseVorbisTime(metadata->samplerate_hz, value);
    }
}

// Parse a vorbis comments structure, reading from the given file.
static void ParseVorbisComments(file_metadata_t *metadata, FILE *fs)
{
    uint32_t buf;
    unsigned int num_comments, i, comment_len;
    char *comment;

    // We must have read the sample rate already from an earlier header.
    if (metadata->samplerate_hz == 0)
    {
	return;
    }

    // Skip the starting part we don't care about.
    if (fread(&buf, 4, 1, fs) < 1)
    {
        return;
    }
    if (fseek(fs, LONG(buf), SEEK_CUR) != 0)
    {
	return;
    }

    // Read count field for number of comments.
    if (fread(&buf, 4, 1, fs) < 1)
    {
        return;
    }
    num_comments = LONG(buf);

    // Read each individual comment.
    for (i = 0; i < num_comments; ++i)
    {
        // Read length of comment.
        if (fread(&buf, 4, 1, fs) < 1)
	{
            return;
	}

        comment_len = LONG(buf);

        // Read actual comment data into string buffer.
        comment = calloc(1, comment_len + 1);
        if (comment == NULL
         || fread(comment, 1, comment_len, fs) < comment_len)
        {
            free(comment);
            break;
        }

        // Parse comment string.
        ParseVorbisComment(metadata, comment);
        free(comment);
    }
}

static void ParseFlacStreaminfo(file_metadata_t *metadata, FILE *fs)
{
    byte buf[34];

    // Read block data.
    if (fread(buf, sizeof(buf), 1, fs) < 1)
    {
        return;
    }

    // We only care about sample rate and song length.
    metadata->samplerate_hz = (buf[10] << 12) | (buf[11] << 4)
                            | (buf[12] >> 4);
    // Song length is actually a 36 bit field, but 32 bits should be
    // enough for everybody.
    //metadata->song_length = (buf[14] << 24) | (buf[15] << 16)
    //                      | (buf[16] << 8) | buf[17];
}

static void ParseFlacFile(file_metadata_t *metadata, FILE *fs)
{
    byte header[4];
    unsigned int block_type;
    size_t block_len;
    boolean last_block;

    for (;;)
    {
        long pos = -1;

        // Read METADATA_BLOCK_HEADER:
        if (fread(header, 4, 1, fs) < 1)
        {
            return;
        }

        block_type = header[0] & ~0x80;
        last_block = (header[0] & 0x80) != 0;
        block_len = (header[1] << 16) | (header[2] << 8) | header[3];

        pos = ftell(fs);
        if (pos < 0)
        {
            return;
        }

        if (block_type == FLAC_STREAMINFO)
        {
            ParseFlacStreaminfo(metadata, fs);
        }
        else if (block_type == FLAC_VORBIS_COMMENT)
        {
            ParseVorbisComments(metadata, fs);
        }

        if (last_block)
        {
            break;
        }

        // Seek to start of next block.
        if (fseek(fs, pos + block_len, SEEK_SET) != 0)
        {
            return;
        }
    }
}

static void ParseOggIdHeader(file_metadata_t *metadata, FILE *fs)
{
    byte buf[21];

    if (fread(buf, sizeof(buf), 1, fs) < 1)
    {
        return;
    }

    metadata->samplerate_hz = (buf[8] << 24) | (buf[7] << 16)
                            | (buf[6] << 8) | buf[5];
}

static void ParseOggFile(file_metadata_t *metadata, FILE *fs)
{
    byte buf[7];
    unsigned int offset;

    // Scan through the start of the file looking for headers. They
    // begin '[byte]vorbis' where the byte value indicates header type.
    memset(buf, 0, sizeof(buf));

    for (offset = 0; offset < 100 * 1024; ++offset)
    {
	// buf[] is used as a sliding window. Each iteration, we
	// move the buffer one byte to the left and read an extra
	// byte onto the end.
        memmove(buf, buf + 1, sizeof(buf) - 1);

        if (fread(&buf[6], 1, 1, fs) < 1)
        {
            return;
        }

        if (!memcmp(buf + 1, "vorbis", 6))
        {
            switch (buf[0])
            {
                case OGG_ID_HEADER:
                    ParseOggIdHeader(metadata, fs);
                    break;
                case OGG_COMMENT_HEADER:
		    ParseVorbisComments(metadata, fs);
                    break;
                default:
                    break;
            }
        }
    }
}

static void ReadLoopPoints(const char *filename, file_metadata_t *metadata)
{
    FILE *fs;
    char header[4];

    metadata->valid = false;
    metadata->samplerate_hz = 0;
    metadata->start_time = 0;
    metadata->end_time = -1;

    fs = fopen(filename, "rb");

    if (fs == NULL)
    {
        return;
    }

    // Check for a recognized file format; use the first four bytes
    // of the file.

    if (fread(header, 4, 1, fs) < 1)
    {
        fclose(fs);
        return;
    }

    if (memcmp(header, FLAC_HEADER, 4) == 0)
    {
        ParseFlacFile(metadata, fs);
    }
    else if (memcmp(header, OGG_HEADER, 4) == 0)
    {
        ParseOggFile(metadata, fs);
    }

    fclose(fs);

    // Only valid if at the very least we read the sample rate.
    metadata->valid = metadata->samplerate_hz > 0;

    // If start and end time are both zero, ignore the loop tags.
    // This is consistent with other source ports.
    if (metadata->start_time == 0 && metadata->end_time == 0)
    {
        metadata->valid = false;
    }
}

// Given a MUS lump, look up a substitute MUS file to play instead
// (or NULL to just use normal MIDI playback).

static const char *GetSubstituteMusicFile(void *data, size_t data_len)
{
    sha1_context_t context;
    sha1_digest_t hash;
    const char *filename;
    char hash_str[sizeof(sha1_digest_t) * 2 + 1];
    unsigned int i;

    // Don't bother doing a hash if we're never going to find anything.
    if (subst_music_len == 0)
    {
        return NULL;
    }

    SHA1_Init(&context);
    SHA1_Update(&context, data, data_len);
    SHA1_Final(hash, &context);

    // Build a string representation of the hash.
    for (i = 0; i < sizeof(sha1_digest_t); ++i)
    {
        M_snprintf(hash_str + i * 2, sizeof(hash_str) - i * 2,
                   "%02x", hash[i]);
    }

    // Look for a hash that matches.
    // The substitute mapping list can (intentionally) contain multiple
    // filename mappings for the same hash. This allows us to try
    // different files and fall back if our first choice isn't found.

    filename = NULL;

    for (i = 0; i < subst_music_len; ++i)
    {
        if (M_StringStartsWith(hash_str, subst_music[i].hash_prefix))
        {
            filename = subst_music[i].filename;

            // If the file exists, then use this file in preference to
            // any fallbacks. But we always return a filename if it's
            // in the list, even if it's just so we can print an error
            // message to the user saying it doesn't exist.
            if (M_FileExists(filename))
            {
                break;
            }
        }
    }

    return filename;
}

static char *GetFullPath(const char *musicdir, const char *path)
{
    char *result;
    char *systemized_path;

    // Starting with directory separator means we have an absolute path,
    // so just return it.
    if (path[0] == DIR_SEPARATOR)
    {
        return M_StringDuplicate(path);
    }

#ifdef _WIN32
    // d:\path\...
    if (isalpha(path[0]) && path[1] == ':' && path[2] == DIR_SEPARATOR)
    {
        return M_StringDuplicate(path);
    }
#endif

    // Paths in the substitute filenames can contain Unix-style /
    // path separators, but we should convert this to the separator
    // for the native platform.
    systemized_path = M_StringReplace(path, "/", DIR_SEPARATOR_S);

    // Copy config filename and cut off the filename to just get the
    // parent dir.
    result = M_StringJoin(musicdir, systemized_path, NULL);
    free(systemized_path);

    return result;
}

// If filename ends with .{ext}, check if a .ogg, .flac or .mp3 exists with
// that name, returning it if found. If none exist, NULL is returned. If the
// filename doesn't end with .{ext} then it just acts as a wrapper around
// GetFullPath().
static char *ExpandFileExtension(const char *musicdir, const char *filename)
{
    static const char *extns[] = {".flac", ".ogg", ".mp3"};
    char *replaced, *result;
    int i;

    if (!M_StringEndsWith(filename, ".{ext}"))
    {
        return GetFullPath(musicdir, filename);
    }

    for (i = 0; i < arrlen(extns); ++i)
    {
        replaced = M_StringReplace(filename, ".{ext}", extns[i]);
        result = GetFullPath(musicdir, replaced);
        free(replaced);
        if (M_FileExists(result))
        {
            return result;
        }
        free(result);
    }

    return NULL;
}

// Add a substitute music file to the lookup list.
static void AddSubstituteMusic(const char *musicdir, const char *hash_prefix,
                               const char *filename)
{
    subst_music_t *s;
    char *path;

    path = ExpandFileExtension(musicdir, filename);
    if (path == NULL)
    {
        return;
    }

    ++subst_music_len;
    subst_music =
        I_Realloc(subst_music, sizeof(subst_music_t) * subst_music_len);
    s = &subst_music[subst_music_len - 1];
    s->hash_prefix = hash_prefix;
    s->filename = path;
}

static const char *ReadHashPrefix(char *line)
{
    char *result;
    char *p;
    int i, len;

    for (p = line; *p != '\0' && !isspace(*p) && *p != '='; ++p)
    {
        if (!isxdigit(*p))
        {
            return NULL;
        }
    }

    len = p - line;
    if (len == 0 || len > sizeof(sha1_digest_t) * 2)
    {
        return NULL;
    }

    result = malloc(len + 1);
    if (result == NULL)
    {
        return NULL;
    }

    for (i = 0; i < len; ++i)
    {
        result[i] = tolower(line[i]);
    }
    result[len] = '\0';

    return result;
}

// Parse a line from substitute music configuration file; returns error
// message or NULL for no error.

static const char *ParseSubstituteLine(char *musicdir, char *line)
{
    const char *hash_prefix;
    char *filename;
    char *p;

    // Strip out comments if present.
    p = strchr(line, '#');
    if (p != NULL)
    {
        while (p > line && isspace(*(p - 1)))
        {
            --p;
        }
        *p = '\0';
    }

    // Skip leading spaces.
    for (p = line; *p != '\0' && isspace(*p); ++p);

    // Empty line? This includes comment lines now that comments have
    // been stripped.
    if (*p == '\0')
    {
        return NULL;
    }

    hash_prefix = ReadHashPrefix(p);
    if (hash_prefix == NULL)
    {
        return "Invalid hash prefix";
    }

    p += strlen(hash_prefix);

    // Skip spaces.
    for (; *p != '\0' && isspace(*p); ++p);

    if (*p != '=')
    {
        return "Expected '='";
    }

    ++p;

    // Skip spaces.
    for (; *p != '\0' && isspace(*p); ++p);

    filename = p;

    // We're now at the filename. Cut off trailing space characters.
    while (strlen(p) > 0 && isspace(p[strlen(p) - 1]))
    {
        p[strlen(p) - 1] = '\0';
    }

    if (strlen(p) == 0)
    {
        return "No filename specified for music substitution";
    }

    // Expand full path and add to our database of substitutes.
    AddSubstituteMusic(musicdir, hash_prefix, filename);

    return NULL;
}

// Read a substitute music configuration file.

static boolean ReadSubstituteConfig(char *musicdir, const char *filename)
{
    char *buffer;
    char *line;
    int linenum = 1;

    // This unnecessarily opens the file twice...
    if (!M_FileExists(filename))
    {
        return false;
    }

    M_ReadFile(filename, (byte **) &buffer);

    line = buffer;

    while (line != NULL)
    {
        const char *error;
        char *next;

        // find end of line
        char *eol = strchr(line, '\n');
        if (eol != NULL)
        {
            // change the newline into NUL
            *eol = '\0';
            next = eol + 1;
        }
        else
        {
            // end of buffer
            next = NULL;
        }

        error = ParseSubstituteLine(musicdir, line);

        if (error != NULL)
        {
            fprintf(stderr, "%s:%i: Error: %s\n", filename, linenum, error);
        }

        ++linenum;
        line = next;
    }

    Z_Free(buffer);

    return true;
}

// Find substitute configs and try to load them.

static void LoadSubstituteConfigs(void)
{
    glob_t *glob;
    char *musicdir;
    const char *path;
    unsigned int old_music_len;
    unsigned int i;

    // We can configure the path to music packs using the music_pack_path
    // configuration variable. Otherwise we use the current directory, or
    // $configdir/music to look for .cfg files.
    if (strcmp(music_pack_path, "") != 0)
    {
        musicdir = M_StringJoin(music_pack_path, DIR_SEPARATOR_S, NULL);
    }
    else if (!strcmp(configdir, exedir))
    {
        musicdir = M_StringDuplicate("");
    }
    else
    {
        musicdir = M_StringJoin(configdir, "music", DIR_SEPARATOR_S, NULL);
    }

    // Load all music packs, by searching for .cfg files.
    glob = I_StartGlob(musicdir, "*.cfg", GLOB_FLAG_SORTED|GLOB_FLAG_NOCASE);
    for (;;)
    {
        path = I_NextGlob(glob);
        if (path == NULL)
        {
            break;
        }
        ReadSubstituteConfig(musicdir, path);
    }
    I_EndGlob(glob);

    if (subst_music_len > 0)
    {
        printf("Loaded %u music substitutions from config files.\n",
               subst_music_len);
    }

    old_music_len = subst_music_len;

    // Add entries from known filenames list. We add this after those from the
    // configuration files, so that the entries here can be overridden.
    for (i = 0; i < arrlen(known_filenames); ++i)
    {
        AddSubstituteMusic(musicdir, known_filenames[i].hash_prefix,
                           known_filenames[i].filename);
    }

    if (subst_music_len > old_music_len)
    {
        printf("Configured %u music substitutions based on filename.\n",
               subst_music_len - old_music_len);
    }

    free(musicdir);
}

// Returns true if the given lump number is a music lump that should
// be included in substitute configs.
// Identifying music lumps by name is not feasible; some games (eg.
// Heretic, Hexen) don't have a common naming pattern for music lumps.

static boolean IsMusicLump(int lumpnum)
{
    byte *data;
    boolean result;

    if (W_LumpLength(lumpnum) < 4)
    {
        return false;
    }

    data = W_CacheLumpNum(lumpnum, PU_STATIC);

    result = memcmp(data, MUS_HEADER_MAGIC, 4) == 0
          || memcmp(data, MID_HEADER_MAGIC, 4) == 0;

    W_ReleaseLumpNum(lumpnum);

    return result;
}

// Dump an example config file containing checksums for all MIDI music
// found in the WAD directory.

static void DumpSubstituteConfig(const char *filename)
{
    sha1_context_t context;
    sha1_digest_t digest;
    char name[9];
    byte *data;
    FILE *fs;
    unsigned int lumpnum;
    size_t h;

    fs = fopen(filename, "w");

    if (fs == NULL)
    {
        I_Error("Failed to open %s for writing", filename);
        return;
    }

    fprintf(fs, "# Example %s substitute MIDI file.\n\n", PACKAGE_NAME);
    fprintf(fs, "# SHA1 hash                              = filename\n");

    for (lumpnum = 0; lumpnum < numlumps; ++lumpnum)
    {
        strncpy(name, lumpinfo[lumpnum]->name, 8);
        name[8] = '\0';

        if (!IsMusicLump(lumpnum))
        {
            continue;
        }

        // Calculate hash.
        data = W_CacheLumpNum(lumpnum, PU_STATIC);
        SHA1_Init(&context);
        SHA1_Update(&context, data, W_LumpLength(lumpnum));
        SHA1_Final(digest, &context);
        W_ReleaseLumpNum(lumpnum);

        // Print line.
        for (h = 0; h < sizeof(sha1_digest_t); ++h)
        {
            fprintf(fs, "%02x", digest[h]);
        }

        fprintf(fs, " = %s.ogg\n", name);
    }

    fprintf(fs, "\n");
    fclose(fs);

    printf("Substitute MIDI config file written to %s.\n", filename);
    I_Quit();
}

// Shutdown music

static void I_MP_ShutdownMusic(void)
{
    if (music_initialized)
    {
        Mix_HaltMusic();
        music_initialized = false;

        if (sdl_was_initialized)
        {
            Mix_CloseAudio();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            sdl_was_initialized = false;
        }
    }
}

static boolean SDLIsInitialized(void)
{
    int freq, channels;
    Uint16 format;

    return Mix_QuerySpec(&freq, &format, &channels) != 0;
}

// Callback function that is invoked to track current track position.
void TrackPositionCallback(int chan, void *stream, int len, void *udata)
{
    // Position is doubled up twice: for 16-bit samples and for stereo.
    current_track_pos += len / 4;
}

// Initialize music subsystem
static boolean I_MP_InitMusic(void)
{
    int i;

    //!
    // @category obscure
    // @arg <filename>
    //
    // Read all MIDI files from loaded WAD files, dump an example substitution
    // music config file to the specified filename and quit.
    //
    i = M_CheckParmWithArgs("-dumpsubstconfig", 1);

    if (i > 0)
    {
        DumpSubstituteConfig(myargv[i + 1]);
    }

    // If we're in GENMIDI mode, try to load sound packs.
    LoadSubstituteConfigs();

    // We can't initialize if we don't have any substitute files to work with.
    // If so, don't bother with SDL initialization etc.
    if (subst_music_len == 0)
    {
        return false;
    }

    // If SDL_mixer is not initialized, we have to initialize it
    // and have the responsibility to shut it down later on.
    if (SDLIsInitialized())
    {
        music_initialized = true;
    }
    else if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Unable to set up sound.\n");
    }
    else if (Mix_OpenAudioDevice(snd_samplerate, AUDIO_S16SYS, 2, 1024, NULL, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) < 0)
    {
        fprintf(stderr, "Error initializing SDL_mixer: %s\n",
                Mix_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    else
    {
        SDL_PauseAudio(0);

        sdl_was_initialized = true;
        music_initialized = true;
    }

    // Initialize SDL_Mixer for digital music playback
    Mix_Init(MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_MP3);

    // Register an effect function to track the music position.
    Mix_RegisterEffect(MIX_CHANNEL_POST, TrackPositionCallback, NULL, NULL);

    return music_initialized;
}

// Set music volume (0 - 127)

static void I_MP_SetMusicVolume(int volume)
{
    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 127);
}

// Start playing a mid

static void I_MP_PlaySong(void *handle, boolean looping)
{
    int loops;

    if (!music_initialized)
    {
        return;
    }

    if (handle == NULL)
    {
        return;
    }

    current_track_music = (Mix_Music *) handle;
    current_track_loop = looping;

    if (looping)
    {
        loops = -1;
    }
    else
    {
        loops = 1;
    }

    // Don't loop when playing substitute music, as we do it
    // ourselves instead.
    if (file_metadata.valid)
    {
        loops = 1;
        SDL_LockAudio();
        current_track_pos = 0;  // start of track
        SDL_UnlockAudio();
    }

    if (Mix_PlayMusic(current_track_music, loops) == -1)
    {
        fprintf(stderr, "I_MP_PlaySong: Error starting track: %s\n",
                Mix_GetError());
    }
}

static void I_MP_PauseSong(void)
{
    if (!music_initialized)
    {
        return;
    }

    Mix_PauseMusic();
}

static void I_MP_ResumeSong(void)
{
    if (!music_initialized)
    {
        return;
    }

    Mix_ResumeMusic();
}

static void I_MP_StopSong(void)
{
    if (!music_initialized)
    {
        return;
    }

    Mix_HaltMusic();
    current_track_music = NULL;
}

static void I_MP_UnRegisterSong(void *handle)
{
    Mix_Music *music = (Mix_Music *) handle;

    if (!music_initialized)
    {
        return;
    }

    if (handle == NULL)
    {
        return;
    }

    Mix_FreeMusic(music);
}

static void *I_MP_RegisterSong(void *data, int len)
{
    const char *filename;
    Mix_Music *music;

    if (!music_initialized)
    {
        return NULL;
    }

    // See if we're substituting this MUS for a high-quality replacement.
    filename = GetSubstituteMusicFile(data, len);
    if (filename == NULL)
    {
        return NULL;
    }

    music = Mix_LoadMUS(filename);
    if (music == NULL)
    {
        // Fall through and play MIDI normally, but print an error
        // message.
        fprintf(stderr, "Failed to load substitute music file: %s: %s\n",
                filename, Mix_GetError());
        return NULL;
    }

    // Read loop point metadata from the file so that we know where
    // to loop the music.
    ReadLoopPoints(filename, &file_metadata);
    return music;
}

// Is the song playing?
static boolean I_MP_MusicIsPlaying(void)
{
    if (!music_initialized)
    {
        return false;
    }

    return Mix_PlayingMusic();
}

// Get position in substitute music track, in seconds since start of track.
static double GetMusicPosition(void)
{
    unsigned int music_pos;
    int freq;

    Mix_QuerySpec(&freq, NULL, NULL);

    SDL_LockAudio();
    music_pos = current_track_pos;
    SDL_UnlockAudio();

    return (double) music_pos / freq;
}

static void RestartCurrentTrack(void)
{
    double start = (double) file_metadata.start_time
                 / file_metadata.samplerate_hz;

    // If the track finished we need to restart it.
    if (current_track_music != NULL)
    {
        Mix_PlayMusic(current_track_music, 1);
    }

    Mix_SetMusicPosition(start);
    SDL_LockAudio();
    current_track_pos = file_metadata.start_time;
    SDL_UnlockAudio();
}

// Poll music position; if we have passed the loop point end position
// then we need to go back.
static void I_MP_PollMusic(void)
{
    // When playing substitute tracks, loop tags only apply if we're playing
    // a looping track. Tracks like the title screen music have the loop
    // tags ignored.
    if (current_track_loop && file_metadata.valid)
    {
        double end = (double) file_metadata.end_time
                   / file_metadata.samplerate_hz;

        // If we have reached the loop end point then we have to take action.
        if (file_metadata.end_time >= 0 && GetMusicPosition() >= end)
        {
            RestartCurrentTrack();
        }

        // Have we reached the actual end of track (not loop end)?
        if (!Mix_PlayingMusic())
        {
            RestartCurrentTrack();
        }
    }
}

music_module_t music_pack_module =
{
    NULL,
    0,
    I_MP_InitMusic,
    I_MP_ShutdownMusic,
    I_MP_SetMusicVolume,
    I_MP_PauseSong,
    I_MP_ResumeSong,
    I_MP_RegisterSong,
    I_MP_UnRegisterSong,
    I_MP_PlaySong,
    I_MP_StopSong,
    I_MP_MusicIsPlaying,
    I_MP_PollMusic,
};

