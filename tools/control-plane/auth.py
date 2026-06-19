"""Authentication for HERMES control plane."""

from __future__ import annotations

import hashlib
import hmac
import os
import secrets
import time

DEFAULT_TOKEN = "hermes-dev"
SECRET = os.environ.get("HERMES_SECRET", "hermes-v0.1-alpha").encode()
_active_tokens: dict[str, float] = {}


def issue_token(username: str = "admin") -> str:
    raw = f"{username}:{secrets.token_hex(16)}:{time.time()}"
    token = hmac.new(SECRET, raw.encode(), hashlib.sha256).hexdigest()
    _active_tokens[token] = time.time() + 86400
    return token


def validate_token(token: str | None) -> bool:
    if not token:
        return False
    if token == os.environ.get("HERMES_TOKEN", DEFAULT_TOKEN):
        return True
    expiry = _active_tokens.get(token)
    if expiry and expiry > time.time():
        return True
    return False


def extract_token(headers) -> str | None:
    auth = headers.get("Authorization", "")
    if auth.startswith("Bearer "):
        return auth[7:].strip()
    return headers.get("X-Hermes-Token")
