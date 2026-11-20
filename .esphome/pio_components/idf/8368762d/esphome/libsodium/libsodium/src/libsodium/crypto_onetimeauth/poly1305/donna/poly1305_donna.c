
#include <string.h>

#include <sodium/sodium_esphome.h>

#include "poly1305_donna.h"
#include "crypto_verify_16.h"
#include "private/common.h"
#include "utils.h"

#ifdef HAVE_TI_MODE
#include "poly1305_donna64.h"
#else
#include "poly1305_donna32.h"
#endif
#include "../onetimeauth_poly1305.h"

static void
poly1305_update(poly1305_state_internal_t *st, const unsigned char *m,
                unsigned long long bytes)
{
    /* handle leftover */
    if (st->leftover) {
        unsigned long long want = (poly1305_block_size - st->leftover);

        if (want > bytes) {
            want = bytes;
        }
        memcpy(&st->buffer[st->leftover], m, (size_t) want);
        bytes -= want;
        m += want;
        st->leftover += want;
        if (st->leftover < poly1305_block_size) {
            return;
        }
        poly1305_blocks(st, st->buffer, poly1305_block_size);
        st->leftover = 0;
    }

    /* process full blocks */
    if (bytes >= poly1305_block_size) {
        unsigned long long want = (bytes & ~(poly1305_block_size - 1));

        poly1305_blocks(st, m, want);
        m += want;
        bytes -= want;
    }

    /* store leftover */
    if (bytes) {
        memcpy(&st->buffer[st->leftover], m, (size_t) bytes);
        st->leftover += bytes;
    }
}

static int
crypto_onetimeauth_poly1305_donna(unsigned char *out, const unsigned char *m,
                                  unsigned long long   inlen,
                                  const unsigned char *key)
{
    CRYPTO_ALIGN(64) poly1305_state_internal_t state;

    poly1305_init(&state, key);
    poly1305_update(&state, m, inlen);
    poly1305_finish(&state, out);

    return 0;
}

static int
crypto_onetimeauth_poly1305_donna_init(crypto_onetimeauth_poly1305_state *state,
                                       const unsigned char *key)
{
    COMPILER_ASSERT(sizeof(crypto_onetimeauth_poly1305_state) >=
        sizeof(poly1305_state_internal_t));
    poly1305_init((poly1305_state_internal_t *) (void *) state, key);

    return 0;
}

static int
crypto_onetimeauth_poly1305_donna_update(
    crypto_onetimeauth_poly1305_state *state, const unsigned char *in,
    unsigned long long inlen)
{
    poly1305_update((poly1305_state_internal_t *) (void *) state, in, inlen);

    return 0;
}

static int
crypto_onetimeauth_poly1305_donna_final(
    crypto_onetimeauth_poly1305_state *state, unsigned char *out)
{
    poly1305_finish((poly1305_state_internal_t *) (void *) state, out);

    return 0;
}

static int
crypto_onetimeauth_poly1305_donna_verify(const unsigned char *h,
                                         const unsigned char *in,
                                         unsigned long long   inlen,
                                         const unsigned char *k)
{
    unsigned char correct[16];

    crypto_onetimeauth_poly1305_donna(correct, in, inlen, k);

    return crypto_verify_16(h, correct);
}

/*
 * ESPHome extension: one-pass Poly1305 over the ChaCha20-Poly1305 AEAD
 * transcript (ad, zero padding, ciphertext, zero padding, then the two
 * 64-bit lengths), mirroring the MAC computed by
 * crypto_aead_chacha20poly1305_ietf_encrypt_detached(). Calls the donna
 * implementation directly instead of the dispatched init/update/final
 * sequence.
 */
int
crypto_onetimeauth_poly1305_aead_mac(unsigned char *mac,
                                     const unsigned char *ad,
                                     unsigned long long adlen,
                                     const unsigned char *c,
                                     unsigned long long clen,
                                     const unsigned char *key)
{
    CRYPTO_ALIGN(64) poly1305_state_internal_t st;
    CRYPTO_ALIGN(16) unsigned char             tail[32];
    size_t                                     rem;
    size_t                                     pad;

    poly1305_init(&st, key);
    if (adlen > 0U) {
        poly1305_update(&st, ad, adlen);
        /* the zero pad comes from tail, which nothing writes until the
           ciphertext step below, rather than from a static the ESP8266
           would keep in DRAM */
        memset(tail, 0, 16);
        poly1305_update(&st, tail, (size_t) ((0x10 - adlen) & 0xf));
    }
    /*
     * Process the ciphertext in whole blocks, then hash the remainder, the
     * zero padding and both lengths from one aligned buffer in a single
     * update. st.leftover stays zero throughout because the ad was padded
     * to a block boundary above.
     */
    rem = (size_t) (clen & 15U);
    poly1305_update(&st, c, clen - rem);
    memcpy(tail, c + (clen - rem), rem);
    pad = (size_t) ((0x10 - clen) & 0xf);
    memset(tail + rem, 0, pad);
    STORE64_LE(tail + rem + pad, adlen);
    STORE64_LE(tail + rem + pad + 8, clen);
    poly1305_update(&st, tail, rem + pad + 16U);
    /* poly1305_finish wipes the state */
    poly1305_finish(&st, mac);

    return 0;
}

struct crypto_onetimeauth_poly1305_implementation
    crypto_onetimeauth_poly1305_donna_implementation = {
        SODIUM_C99(.onetimeauth =) crypto_onetimeauth_poly1305_donna,
        SODIUM_C99(.onetimeauth_verify =)
            crypto_onetimeauth_poly1305_donna_verify,
        SODIUM_C99(.onetimeauth_init =) crypto_onetimeauth_poly1305_donna_init,
        SODIUM_C99(.onetimeauth_update =)
            crypto_onetimeauth_poly1305_donna_update,
        SODIUM_C99(.onetimeauth_final =) crypto_onetimeauth_poly1305_donna_final
    };
