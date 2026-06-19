# HERMES Security

Security layer status: **alpha** — suitable for development and demos; production deployments need key management hardening.

| Component | Path |
|-----------|------|
| C library | `security/` |
| Control plane auth | `tools/control-plane/auth.py` |
| Package signing | `sdk/package.py` |

---

## Threat model (v0.1)

| Protected | Not yet protected |
|-----------|-------------------|
| API access via token | Per-node TLS on UART |
| HMAC-signed app packages | Hardware secure element |
| SHA256 binary integrity | Rate limiting / RBAC |
| Identity challenge (C API) | OAuth / JWT federation |

---

## Control plane authentication

### Dev token (default)

```
X-Hermes-Token: hermes-dev
```

Or `Authorization: Bearer hermes-dev`

Set via environment:

```bash
export HERMES_TOKEN=my-production-token
export HERMES_SECRET=my-hmac-secret-key
```

### Login flow

```bash
curl -X POST http://localhost:8787/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin"}'
# → {"token":"<64-char-hex>"}
```

`issue_token()` in `auth.py`:

1. Generates `username:random:timestamp`
2. Signs with `HMAC-SHA256(HERMES_SECRET, raw)`
3. Stores token with 24h expiry in memory

`validate_token()` accepts:
- `HERMES_TOKEN` env match
- Active issued token not expired

### Public endpoints (no auth)

- `GET /api/status`
- `POST /api/auth/login`
- Static dashboard assets (`/`, `/style.css`, `/app.js`)

All other `/api/*` routes require a valid token.

---

## Package signing (deploy pipeline)

When packaging an app:

```bash
python sdk/package.py examples/smartfarm host
```

Output in `examples/smartfarm/binaries/`:

| File | Content |
|------|---------|
| `smartfarm.bin` | Built binary |
| `manifest.json` | App metadata |
| `smartfarm.sha256` | SHA256 digest |
| `smartfarm.sig` | HMAC-SHA256 signature |

Signing uses `HERMES_SECRET` (same as control plane). Node agents verify before flash to `data/flash/`.

### C API

```c
// security/include/hermes_sha256.h
int hermes_sha256(const uint8_t *data, size_t len, uint8_t out[32]);

// security/include/hermes_hmac.h
int hermes_hmac_sha256(const uint8_t *key, size_t key_len,
                       const uint8_t *data, size_t len, uint8_t out[32]);
```

---

## Node identity (C)

```c
typedef struct {
    char node_id[32];
    uint8_t public_key[32];
    uint8_t node_type;
    bool authenticated;
} hermes_identity_t;
```

| Function | Purpose |
|----------|---------|
| `hermes_identity_init()` | Bind node ID and type |
| `hermes_identity_challenge()` | Generate 16-byte challenge |
| `hermes_identity_authenticate()` | Verify challenge response |
| `hermes_identity_verify_peer()` | Mutual authentication |

Used in cluster link setup before frame routing. Challenge-response prevents unauthenticated nodes from joining.

---

## Policy engine

```c
// security/include/hermes_policy.h
int hermes_policy_check(const char *node_id, const char *action, const char *resource);
```

Default policy (alpha): allow all actions for authenticated nodes. Production: load rules from `security/policy.json` (planned v0.2).

### App verification flow

```
Install package → hermes_verify_app() → SHA256 + HMAC check → hermes_policy_check() → Run
```

```c
typedef struct {
    char app_id[32];
    char version[16];
    uint8_t signature[64];
    uint32_t payload_size;
} hermes_signed_app_t;
```

---

## Dev vs production checklist

| Setting | Development | Production |
|---------|-------------|------------|
| `HERMES_TOKEN` | `hermes-dev` | Random 32+ byte secret |
| `HERMES_SECRET` | default string | Unique per deployment |
| API exposure | localhost:8787 | Reverse proxy + TLS |
| Serial bridge | USB COM port | Encrypted tunnel or VPN |
| Package signing | Optional | Required before OTA |
| Identity | Simulated | Per-node keypair in flash |

---

## Related

- [API.md](API.md) — auth headers and login endpoint
- [SDK_GUIDE.md](SDK_GUIDE.md) — package and deploy workflow
- [RELEASE_v0.1.md](RELEASE_v0.1.md) — known security limitations
