#ifndef HERMES_HMAC_H
#define HERMES_HMAC_H

#include <stddef.h>
#include <stdint.h>

#define HERMES_HMAC_SIZE 32

void hermes_hmac_sha256(const uint8_t *key, size_t key_len,
                        const uint8_t *data, size_t data_len,
                        uint8_t out[HERMES_HMAC_SIZE]);

int hermes_hmac_verify(const uint8_t *key, size_t key_len,
                       const uint8_t *data, size_t data_len,
                       const uint8_t *expected, size_t expected_len);

#endif
