#ifndef HERMES_IDENTITY_H
#define HERMES_IDENTITY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HERMES_NODE_ID_LEN   32
#define HERMES_SIGNATURE_LEN 64

typedef struct {
    char node_id[HERMES_NODE_ID_LEN];
    uint8_t public_key[32];
    uint8_t node_type;
    bool authenticated;
} hermes_identity_t;

typedef struct {
    char app_id[32];
    char version[16];
    uint8_t signature[HERMES_SIGNATURE_LEN];
    uint32_t payload_size;
} hermes_signed_app_t;

int  hermes_identity_init(hermes_identity_t *id, const char *node_id, uint8_t node_type);
int  hermes_identity_authenticate(hermes_identity_t *id, const uint8_t *challenge,
                                  const uint8_t *response, size_t len);
int  hermes_identity_verify_peer(const hermes_identity_t *local,
                                 const hermes_identity_t *peer);
int  hermes_identity_challenge(uint8_t challenge[16]);

#endif /* HERMES_IDENTITY_H */
