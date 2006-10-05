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

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define UWORD32 DWORD
#else
#include <inttypes.h>
#define UWORD32 uint32_t
#endif
#define md5byte unsigned char

typedef struct md5_context_s md5_context_t;

struct md5_context_s {
        UWORD32 buf[4];
        UWORD32 bytes[2];
        UWORD32 in[16];
};

void MD5_Init(md5_context_t *context);
void MD5_Update(md5_context_t *context, md5byte const *buf, unsigned len);
void MD5_UpdateInt32(md5_context_t *context, unsigned int val);
void MD5_UpdateString(md5_context_t *context, char *str);
void MD5_Final(unsigned char digest[16], md5_context_t *context);
void MD5_Transform(UWORD32 buf[4], UWORD32 const in[16]);

#endif /* !MD5_H */
