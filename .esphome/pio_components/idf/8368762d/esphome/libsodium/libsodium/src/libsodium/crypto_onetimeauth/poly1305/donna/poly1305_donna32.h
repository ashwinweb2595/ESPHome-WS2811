/*
   poly1305 implementation using 32 bit * 32 bit = 64 bit multiplication
   and 64 bit addition

   When the message pointer is 4-byte aligned, uses 4 aligned 32-bit loads
   per block instead of 5 unaligned byte-at-a-time loads. This benefits
   architectures (like Xtensa) where unaligned loads are expensive.
*/

#if defined(_MSC_VER)
# define POLY1305_NOINLINE __declspec(noinline)
#elif defined(__clang__) || defined(__GNUC__)
# define POLY1305_NOINLINE __attribute__((noinline))
#else
# define POLY1305_NOINLINE
#endif

#include "private/common.h"
#include "sodium/esphome_platform.h"
#include <stdint.h>

#define poly1305_block_size 16

#ifdef SODIUM_ESPHOME_ESP8266_PATHS
/* ESPHome: the LX106 has no 32x32->64 multiply, so each int64 product of the
   block loop below is a ROM __umulsidi3 call; four 16-bit partial products
   (mul16u) stay inline. Inputs are below 2^29, the sum of five fits in 64 bits
   as before. The byte-wise loads of the block loop use the inline helper
   from esphome_platform.h; -Os leaves the library ones out of line. */
static inline __attribute__((always_inline)) unsigned long long
poly1305_mul32x32(unsigned long a, unsigned long b)
{
    const uint32_t al = a & 0xffff, ah = a >> 16, bl = b & 0xffff, bh = b >> 16;
    const uint32_t ll = (uint32_t) (uint16_t) al * (uint32_t) (uint16_t) bl;
    const uint32_t lh = (uint32_t) (uint16_t) al * (uint32_t) (uint16_t) bh;
    const uint32_t hl = (uint32_t) (uint16_t) ah * (uint32_t) (uint16_t) bl;
    const uint32_t hh = (uint32_t) (uint16_t) ah * (uint32_t) (uint16_t) bh;
    const uint32_t mid = (ll >> 16) + (lh & 0xffff) + (hl & 0xffff);
    const uint32_t hi  = hh + (lh >> 16) + (hl >> 16) + (mid >> 16);
    const uint32_t lo  = (mid << 16) | (ll & 0xffff);

    return ((unsigned long long) hi << 32) | lo;
}
#define POLY1305_MUL(a, b) poly1305_mul32x32((a), (b))
#define POLY1305_LOAD32_LE(p) sodium_esphome_load32_le(p)
#else
#define POLY1305_MUL(a, b) ((unsigned long long) (a) * (b))
#define POLY1305_LOAD32_LE(p) LOAD32_LE(p)
#endif

/* 17 + sizeof(unsigned long long) + 14*sizeof(unsigned long) */
typedef struct poly1305_state_internal_t {
    unsigned long      r[5];
    unsigned long      h[5];
    unsigned long      pad[4];
    unsigned long long leftover;
    unsigned char      buffer[poly1305_block_size];
    unsigned char      final;
} poly1305_state_internal_t;

/* Load a 32-bit little-endian value from a pointer that is known to be
   4-byte aligned. On little-endian targets this compiles to a single
   aligned load instruction (e.g. l32i on Xtensa). */
static inline __attribute__((always_inline)) uint32_t
load32_le_aligned(const void *p) {
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    return *(const uint32_t *)p;
#else
    const uint8_t *b = (const uint8_t *)p;
    return ((uint32_t)b[0])
         | ((uint32_t)b[1] << 8)
         | ((uint32_t)b[2] << 16)
         | ((uint32_t)b[3] << 24);
#endif
}

static void
poly1305_init(poly1305_state_internal_t *st, const unsigned char key[32])
{
    /* r &= 0xffffffc0ffffffc0ffffffc0fffffff - wiped after finalization */
    if (((uintptr_t) key & 3) == 0) {
        uint32_t k0 = load32_le_aligned(&key[0]);
        uint32_t k1 = load32_le_aligned(&key[4]);
        uint32_t k2 = load32_le_aligned(&key[8]);
        uint32_t k3 = load32_le_aligned(&key[12]);

        st->r[0] = k0 & 0x3ffffff;
        st->r[1] = ((k0 >> 26) | (k1 << 6)) & 0x3ffff03;
        st->r[2] = ((k1 >> 20) | (k2 << 12)) & 0x3ffc0ff;
        st->r[3] = ((k2 >> 14) | (k3 << 18)) & 0x3f03fff;
        st->r[4] = (k3 >> 8) & 0x00fffff;

        /* save pad for later */
        st->pad[0] = load32_le_aligned(&key[16]);
        st->pad[1] = load32_le_aligned(&key[20]);
        st->pad[2] = load32_le_aligned(&key[24]);
        st->pad[3] = load32_le_aligned(&key[28]);
    } else {
        st->r[0] = (POLY1305_LOAD32_LE(&key[0])) & 0x3ffffff;
        st->r[1] = (POLY1305_LOAD32_LE(&key[3]) >> 2) & 0x3ffff03;
        st->r[2] = (POLY1305_LOAD32_LE(&key[6]) >> 4) & 0x3ffc0ff;
        st->r[3] = (POLY1305_LOAD32_LE(&key[9]) >> 6) & 0x3f03fff;
        st->r[4] = (POLY1305_LOAD32_LE(&key[12]) >> 8) & 0x00fffff;

        /* save pad for later */
        st->pad[0] = POLY1305_LOAD32_LE(&key[16]);
        st->pad[1] = POLY1305_LOAD32_LE(&key[20]);
        st->pad[2] = POLY1305_LOAD32_LE(&key[24]);
        st->pad[3] = POLY1305_LOAD32_LE(&key[28]);
    }

    /* h = 0 */
    st->h[0] = 0;
    st->h[1] = 0;
    st->h[2] = 0;
    st->h[3] = 0;
    st->h[4] = 0;

    st->leftover = 0;
    st->final    = 0;
}

static void
poly1305_blocks(poly1305_state_internal_t *st, const unsigned char *m,
                unsigned long long bytes)
{
    const unsigned long hibit = (st->final) ? 0UL : (1UL << 24); /* 1 << 128 */
    const int           aligned = (((uintptr_t) m & 3) == 0);
    unsigned long       r0, r1, r2, r3, r4;
    unsigned long       s1, s2, s3, s4;
    unsigned long       h0, h1, h2, h3, h4;
    unsigned long long  d0, d1, d2, d3, d4;
    unsigned long       c;

    r0 = st->r[0];
    r1 = st->r[1];
    r2 = st->r[2];
    r3 = st->r[3];
    r4 = st->r[4];

    s1 = r1 * 5;
    s2 = r2 * 5;
    s3 = r3 * 5;
    s4 = r4 * 5;

    h0 = st->h[0];
    h1 = st->h[1];
    h2 = st->h[2];
    h3 = st->h[3];
    h4 = st->h[4];

    while (bytes >= poly1305_block_size) {
        /*
         * h += m[i]
         *
         * If m is 4-byte aligned, use 4 aligned 32-bit loads and
         * reconstruct the 26-bit limbs with shifts. Otherwise fall
         * back to the original 5 unaligned byte-at-a-time loads.
         * Alignment is constant for all iterations (m advances by 16).
         */
        if (aligned) {
            uint32_t w0 = load32_le_aligned(m + 0);
            uint32_t w1 = load32_le_aligned(m + 4);
            uint32_t w2 = load32_le_aligned(m + 8);
            uint32_t w3 = load32_le_aligned(m + 12);

            h0 += w0 & 0x3ffffff;
            h1 += ((w0 >> 26) | (w1 << 6)) & 0x3ffffff;
            h2 += ((w1 >> 20) | (w2 << 12)) & 0x3ffffff;
            h3 += ((w2 >> 14) | (w3 << 18)) & 0x3ffffff;
            h4 += (w3 >> 8) | hibit;
        } else {
            h0 += (POLY1305_LOAD32_LE(m + 0)) & 0x3ffffff;
            h1 += (POLY1305_LOAD32_LE(m + 3) >> 2) & 0x3ffffff;
            h2 += (POLY1305_LOAD32_LE(m + 6) >> 4) & 0x3ffffff;
            h3 += (POLY1305_LOAD32_LE(m + 9) >> 6) & 0x3ffffff;
            h4 += (POLY1305_LOAD32_LE(m + 12) >> 8) | hibit;
        }

        /* h *= r */
        d0 = POLY1305_MUL(h0, r0) + POLY1305_MUL(h1, s4) +
             POLY1305_MUL(h2, s3) + POLY1305_MUL(h3, s2) +
             POLY1305_MUL(h4, s1);
        d1 = POLY1305_MUL(h0, r1) + POLY1305_MUL(h1, r0) +
             POLY1305_MUL(h2, s4) + POLY1305_MUL(h3, s3) +
             POLY1305_MUL(h4, s2);
        d2 = POLY1305_MUL(h0, r2) + POLY1305_MUL(h1, r1) +
             POLY1305_MUL(h2, r0) + POLY1305_MUL(h3, s4) +
             POLY1305_MUL(h4, s3);
        d3 = POLY1305_MUL(h0, r3) + POLY1305_MUL(h1, r2) +
             POLY1305_MUL(h2, r1) + POLY1305_MUL(h3, r0) +
             POLY1305_MUL(h4, s4);
        d4 = POLY1305_MUL(h0, r4) + POLY1305_MUL(h1, r3) +
             POLY1305_MUL(h2, r2) + POLY1305_MUL(h3, r1) +
             POLY1305_MUL(h4, r0);

        /* (partial) h %= p */
        c  = (unsigned long) (d0 >> 26);
        h0 = (unsigned long) d0 & 0x3ffffff;
        d1 += c;
        c  = (unsigned long) (d1 >> 26);
        h1 = (unsigned long) d1 & 0x3ffffff;
        d2 += c;
        c  = (unsigned long) (d2 >> 26);
        h2 = (unsigned long) d2 & 0x3ffffff;
        d3 += c;
        c  = (unsigned long) (d3 >> 26);
        h3 = (unsigned long) d3 & 0x3ffffff;
        d4 += c;
        c  = (unsigned long) (d4 >> 26);
        h4 = (unsigned long) d4 & 0x3ffffff;
        h0 += c * 5;
        c  = (h0 >> 26);
        h0 &= 0x3ffffff;
        h1 += c;

        m += poly1305_block_size;
        bytes -= poly1305_block_size;
    }

    st->h[0] = h0;
    st->h[1] = h1;
    st->h[2] = h2;
    st->h[3] = h3;
    st->h[4] = h4;
}

static POLY1305_NOINLINE void
poly1305_finish(poly1305_state_internal_t *st, unsigned char mac[16])
{
    unsigned long      h0, h1, h2, h3, h4, c;
    unsigned long      g0, g1, g2, g3, g4;
    unsigned long long f;
    unsigned long      mask;

    /* process the remaining block */
    if (st->leftover) {
        unsigned long long i = st->leftover;

        st->buffer[i++] = 1;
        for (; i < poly1305_block_size; i++) {
            st->buffer[i] = 0;
        }
        st->final = 1;
        poly1305_blocks(st, st->buffer, poly1305_block_size);
    }

    /* fully carry h */
    h0 = st->h[0];
    h1 = st->h[1];
    h2 = st->h[2];
    h3 = st->h[3];
    h4 = st->h[4];

    c  = h1 >> 26;
    h1 = h1 & 0x3ffffff;
    h2 += c;
    c  = h2 >> 26;
    h2 = h2 & 0x3ffffff;
    h3 += c;
    c  = h3 >> 26;
    h3 = h3 & 0x3ffffff;
    h4 += c;
    c  = h4 >> 26;
    h4 = h4 & 0x3ffffff;
    h0 += c * 5;
    c  = h0 >> 26;
    h0 = h0 & 0x3ffffff;
    h1 += c;

    /* compute h + -p */
    g0 = h0 + 5;
    c  = g0 >> 26;
    g0 &= 0x3ffffff;
    g1 = h1 + c;
    c  = g1 >> 26;
    g1 &= 0x3ffffff;
    g2 = h2 + c;
    c  = g2 >> 26;
    g2 &= 0x3ffffff;
    g3 = h3 + c;
    c  = g3 >> 26;
    g3 &= 0x3ffffff;
    g4 = h4 + c - (1UL << 26);

    /* select h if h < p, or h + -p if h >= p */
    mask = (g4 >> ((sizeof(unsigned long) * 8) - 1)) - 1;
    g0 &= mask;
    g1 &= mask;
    g2 &= mask;
    g3 &= mask;
    g4 &= mask;
    mask = ~mask;

    h0 = (h0 & mask) | g0;
    h1 = (h1 & mask) | g1;
    h2 = (h2 & mask) | g2;
    h3 = (h3 & mask) | g3;
    h4 = (h4 & mask) | g4;

    /* h = h % (2^128) */
    h0 = ((h0) | (h1 << 26)) & 0xffffffff;
    h1 = ((h1 >> 6) | (h2 << 20)) & 0xffffffff;
    h2 = ((h2 >> 12) | (h3 << 14)) & 0xffffffff;
    h3 = ((h3 >> 18) | (h4 << 8)) & 0xffffffff;

    /* mac = (h + pad) % (2^128) */
    f  = (unsigned long long) h0 + st->pad[0];
    h0 = (unsigned long) f;
    f  = (unsigned long long) h1 + st->pad[1] + (f >> 32);
    h1 = (unsigned long) f;
    f  = (unsigned long long) h2 + st->pad[2] + (f >> 32);
    h2 = (unsigned long) f;
    f  = (unsigned long long) h3 + st->pad[3] + (f >> 32);
    h3 = (unsigned long) f;

    STORE32_LE(mac + 0, (uint32_t) h0);
    STORE32_LE(mac + 4, (uint32_t) h1);
    STORE32_LE(mac + 8, (uint32_t) h2);
    STORE32_LE(mac + 12, (uint32_t) h3);

    /* zero out the state */
    sodium_memzero((void *) st, sizeof *st);
}
