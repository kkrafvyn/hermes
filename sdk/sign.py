#!/usr/bin/env python3

"""HMAC signing for HERMES manifests and binaries."""



from __future__ import annotations



import hashlib

import hmac

import os



SECRET = os.environ.get("HERMES_SECRET", "hermes-v0.1-alpha").encode()





def sign_bytes(data: bytes) -> str:

    return hmac.new(SECRET, data, hashlib.sha256).hexdigest()





def verify_bytes(data: bytes, signature: str) -> bool:

    expected = sign_bytes(data)

    return hmac.compare_digest(expected, signature)


