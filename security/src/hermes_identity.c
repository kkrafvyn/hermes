#include "hermes_identity.h"
#include "hermes_hmac.h"
#include <string.h>

static uint8_t g_node_secret[32];

int hermes_identity_init(hermes_identity_t *id, const char *node_id, uint8_t node_type)
{
    if (!id || !node_id) return -1;
    memset(id, 0, sizeof(*id));
    strncpy(id->node_id, node_id, sizeof(id->node_id) - 1);
    id->node_type = node_type;
    hermes_hmac_sha256((const uint8_t *)node_id, strlen(node_id),
                       (const uint8_t *)"hermes-node", 11, g_node_secret);
    return 0;
}

int hermes_identity_authenticate(hermes_identity_t *id, const uint8_t *challenge,
                                 const uint8_t *response, size_t len)
{
    if (!id || !challenge || !response || len < HERMES_HMAC_SIZE) return -1;
    uint8_t expected[HERMES_HMAC_SIZE];
    hermes_hmac_sha256(g_node_secret, sizeof(g_node_secret),
                       challenge, 16, expected);
    id->authenticated = memcmp(expected, response, HERMES_HMAC_SIZE) == 0;
    return id->authenticated ? 0 : -2;
}

int hermes_identity_verify_peer(const hermes_identity_t *local,
                                const hermes_identity_t *peer)
{
    (void)local;
    if (!peer) return -1;
    return peer->authenticated ? 0 : -2;
}

int hermes_identity_challenge(uint8_t challenge[16])
{
    if (!challenge) return -1;
    for (int i = 0; i < 16; i++)
        challenge[i] = (uint8_t)(g_node_secret[i] ^ (uint8_t)(i * 17));
    return 0;
}
