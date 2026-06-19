#include "hermes_hmac.h"
#include "hermes_sha256.h"
#include <string.h>

#define HMAC_BLOCK 64

static void sha256_block(const uint8_t *data, size_t len, uint8_t out[32])
{
    hermes_sha256(data, len, out);
}

void hermes_hmac_sha256(const uint8_t *key, size_t key_len,
                        const uint8_t *data, size_t data_len,
                        uint8_t out[HERMES_HMAC_SIZE])
{
    uint8_t k[HMAC_BLOCK];
    uint8_t ipad[HMAC_BLOCK + 256];
    uint8_t opad[HMAC_BLOCK + 32];
    uint8_t inner[32];

    memset(k, 0, sizeof(k));
    if (key_len > HMAC_BLOCK) {
        hermes_sha256(key, key_len, k);
        key_len = 32;
        key = k;
    } else if (key) {
        memcpy(k, key, key_len);
    }

    for (size_t i = 0; i < HMAC_BLOCK; i++) {
        ipad[i] = (uint8_t)(k[i] ^ 0x36);
        opad[i] = (uint8_t)(k[i] ^ 0x5c);
    }

    memcpy(ipad + HMAC_BLOCK, data, data_len);
    sha256_block(ipad, HMAC_BLOCK + data_len, inner);

    memcpy(opad + HMAC_BLOCK, inner, 32);
    sha256_block(opad, HMAC_BLOCK + 32, out);
}

int hermes_hmac_verify(const uint8_t *key, size_t key_len,
                       const uint8_t *data, size_t data_len,
                       const uint8_t *expected, size_t expected_len)
{
    uint8_t digest[HERMES_HMAC_SIZE];
    if (!expected || expected_len < HERMES_HMAC_SIZE) return -1;
    hermes_hmac_sha256(key, key_len, data, data_len, digest);
    return memcmp(digest, expected, HERMES_HMAC_SIZE) == 0 ? 0 : -2;
}
