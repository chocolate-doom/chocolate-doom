/*
 * This is the header file for the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * md5_context_s structure, pass it to MD5_Init, call MD5_Update as
 * needed on buffers full of bytes, and then call MD5_Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * Changed so as no longer to depend on Colin Plumb's `usual.h'
 * header definitions; now uses stuff from dpkg's config.h
 *  - Ian Jackson <ian@chiark.greenend.org.uk>.
 * Still in the public domain.
 */

#ifndef MD5_H
#define MD5_H

#include "doomtype.h"

typedef struct md5_context_s md5_context_t;
typedef byte md5_digest_t[16];

struct md5_context_s {
        uint32_t buf[4];
        uint32_t bytes[2];
        uint32_t in[16];
};

void MD5_Init(md5_context_t *context);
void MD5_Update(md5_context_t *context, byte const *buf, unsigned len);
void MD5_UpdateInt32(md5_context_t *context, unsigned int val);
void MD5_UpdateString(md5_context_t *context, char *str);
void MD5_Final(unsigned char digest[16], md5_context_t *context);
void MD5_Transform(uint32_t buf[4], uint32_t const in[16]);

#endif /* !MD5_H */

