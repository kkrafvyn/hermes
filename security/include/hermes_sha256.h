#ifndef HERMES_SHA256_H
#define HERMES_SHA256_H

#include <stdint.h>
#include <stddef.h>

#define HERMES_SHA256_DIGEST_SIZE 32

typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;
} hermes_sha256_ctx_t;

void hermes_sha256_init(hermes_sha256_ctx_t *ctx);
void hermes_sha256_update(hermes_sha256_ctx_t *ctx, const uint8_t *data, size_t len);
void hermes_sha256_final(hermes_sha256_ctx_t *ctx, uint8_t digest[HERMES_SHA256_DIGEST_SIZE]);
int  hermes_sha256(const uint8_t *data, size_t len, uint8_t digest[HERMES_SHA256_DIGEST_SIZE]);

#endif
