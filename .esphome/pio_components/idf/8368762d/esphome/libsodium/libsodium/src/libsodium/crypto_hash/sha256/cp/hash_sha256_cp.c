
/*-
 * Copyright 2005,2007,2009 Colin Percival
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include "crypto_hash_sha256.h"
#include "private/common.h"
#include "sodium/esphome_platform.h"
#include "utils.h"

#ifdef SODIUM_ESPHOME_ESP8266_PATHS
/* Big endian block decode without the libgcc byte swap call, as patch 13
   does for the little endian loads */
#undef LOAD32_BE
#define LOAD32_BE(p) sodium_esphome_load32_be(p)
#endif

static void
be32enc_vect(unsigned char *dst, const uint32_t *src, size_t len)
{
    size_t i;

    for (i = 0; i < len / 4; i++) {
        STORE32_BE(dst + i * 4, src[i]);
    }
}

static void
be32dec_vect(uint32_t *dst, const unsigned char *src, size_t len)
{
    size_t i;

    for (i = 0; i < len / 4; i++) {
        dst[i] = LOAD32_BE(src + i * 4);
    }
}

#ifdef SODIUM_ESPHOME_ESP8266
/* The round constants and the initial state are only ever read as whole
   words (l32i), so they can live in flash like the ed25519 base table;
   keeps 288 bytes of DRAM free. Flash only supports aligned 32-bit access,
   so the alignment is stated explicitly rather than left to the array's
   natural alignment. */
#define SHA256_CONST_ATTR CRYPTO_ALIGN(4) __attribute__((section(".irom.text")))
#else
#define SHA256_CONST_ATTR
#endif
static const uint32_t SHA256_CONST_ATTR Krnd[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define Ch(x, y, z) ((x & (y ^ z)) ^ z)
#define Maj(x, y, z) ((x & (y | z)) | (y & z))
#define SHR(x, n) (x >> n)
#define ROTR(x, n) ROTR32(x, n)
#define S0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define s1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

#define RND(a, b, c, d, e, f, g, h, k) \
    h += S1(e) + Ch(e, f, g) + k;      \
    d += h;                            \
    h += S0(a) + Maj(a, b, c);

/* Four rounds per loop pass, the working variables rotated by name inside
   the pass and swapped once at its end, in place of 64 unrolled rounds:
   about 1.9 KB less flash on the ESP8266 (the transform 2508 to 600 bytes)
   at the unrolled transform's speed per block. Fewer rounds per pass are
   smaller still but slower per block, and a handshake, whose hashes come
   in cold from flash, measured fastest with four on every target. The
   working variables are plain locals; the ESP8266 build happens to keep
   them in registers, and a build that spills them leaves no more behind
   than any compiler could have left of upstream's. Upstream's copy of
   them in the caller's scratch, the part its sodium_memzero wiped, is not
   kept. */
static void
SHA256_Transform(uint32_t state[8], const uint8_t block[64], uint32_t W[64])
{
    uint32_t a, b, c, d, e, f, g, h, t;
    int i;

    be32dec_vect(W, block, 64);
    for (i = 16; i < 64; i++) {
        W[i] = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];
    }
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    f = state[5];
    g = state[6];
    h = state[7];
    for (i = 0; i < 64; i += 4) {
        /* RND leaves the new a in its last argument and the new e in its
           fourth, so each round names the variables one step around */
        RND(a, b, c, d, e, f, g, h, W[i] + Krnd[i]);
        RND(h, a, b, c, d, e, f, g, W[i + 1] + Krnd[i + 1]);
        RND(g, h, a, b, c, d, e, f, W[i + 2] + Krnd[i + 2]);
        RND(f, g, h, a, b, c, d, e, W[i + 3] + Krnd[i + 3]);
        t = a; a = e; e = t;
        t = b; b = f; f = t;
        t = c; c = g; g = t;
        t = d; d = h; h = t;
    }
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

static void
SHA256_Pad(crypto_hash_sha256_state *state, uint32_t tmp32[64])
{
    unsigned int r;

    ACQUIRE_FENCE;
    r = (unsigned int) ((state->count >> 3) & 0x3f);
    /* The padding is one 0x80 and zeros to the length field, written
       rather than copied from a table the ESP8266 would keep in DRAM. */
    state->buf[r++] = 0x80;
    if (r > 56) {
        memset(&state->buf[r], 0, 64 - r);
        SHA256_Transform(state->state, state->buf, tmp32);
        r = 0;
    }
    memset(&state->buf[r], 0, 56 - r);
    STORE64_BE(&state->buf[56], state->count);
    SHA256_Transform(state->state, state->buf, tmp32);
}

static const uint32_t SHA256_CONST_ATTR sha256_initial_state[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};
#undef SHA256_CONST_ATTR

int
crypto_hash_sha256_init(crypto_hash_sha256_state *state)
{
    state->count = (uint64_t) 0U;
#ifdef SODIUM_ESPHOME_ESP8266_PATHS
    /* word by word: on the ESP8266 the table is in flash, which memcpy
       would read byte-wise, and the host known answer tests compile this
       path as well; the volatile read keeps the compiler from turning the
       loop back into a memcpy */
    {
        const volatile uint32_t *src = sha256_initial_state;
        int                      i;
        for (i = 0; i < 8; i++) {
            state->state[i] = src[i];
        }
    }
#else
    memcpy(state->state, sha256_initial_state, sizeof sha256_initial_state);
#endif

    return 0;
}

int
crypto_hash_sha256_update(crypto_hash_sha256_state *state,
                          const unsigned char *in, unsigned long long inlen)
{
    uint32_t           tmp32[64];
    unsigned long long i;
    unsigned long long r;

    if (inlen <= 0U) {
        return 0;
    }
    ACQUIRE_FENCE;
    r = (unsigned long long) ((state->count >> 3) & 0x3f);

    state->count += ((uint64_t) inlen) << 3;
    if (inlen < 64 - r) {
        for (i = 0; i < inlen; i++) {
            state->buf[r + i] = in[i];
        }
        return 0;
    }
    for (i = 0; i < 64 - r; i++) {
        state->buf[r + i] = in[i];
    }
    SHA256_Transform(state->state, state->buf, tmp32);
    in += 64 - r;
    inlen -= 64 - r;

    while (inlen >= 64) {
        SHA256_Transform(state->state, in, tmp32);
        in += 64;
        inlen -= 64;
    }
    inlen &= 63;
    for (i = 0; i < inlen; i++) {
        state->buf[i] = in[i];
    }
    sodium_memzero((void *) tmp32, sizeof tmp32);

    return 0;
}

int
crypto_hash_sha256_final(crypto_hash_sha256_state *state, unsigned char *out)
{
    uint32_t tmp32[64];

    SHA256_Pad(state, tmp32);
    be32enc_vect(out, state->state, 32);
    sodium_memzero((void *) tmp32, sizeof tmp32);
    sodium_memzero((void *) state, sizeof *state);

    return 0;
}

int
crypto_hash_sha256(unsigned char *out, const unsigned char *in,
                   unsigned long long inlen)
{
    crypto_hash_sha256_state state;

    crypto_hash_sha256_init(&state);
    crypto_hash_sha256_update(&state, in, inlen);
    crypto_hash_sha256_final(&state, out);

    return 0;
}
