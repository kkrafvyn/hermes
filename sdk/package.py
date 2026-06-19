#!/usr/bin/env python3

"""Package HERMES applications for deploy."""



from __future__ import annotations



import hashlib

import json

import shutil

import sys

from pathlib import Path



ROOT = Path(__file__).resolve().parent.parent



sys.path.insert(0, str(Path(__file__).resolve().parent))

from sign import sign_bytes  # noqa: E402





def package(app_dir: Path, target: str = "host") -> Path:

    app_dir = app_dir.resolve()

    manifest = json.loads((app_dir / "manifest.json").read_text())

    pkg_dir = app_dir / "binaries"

    pkg_dir.mkdir(exist_ok=True)



    build_py = Path(__file__).resolve().parent / "build.py"

    import subprocess

    subprocess.call([sys.executable, str(build_py), str(app_dir), target])



    build_dir = app_dir / "build" / target

    bin_file = build_dir / f"{manifest['name']}.bin"

    if bin_file.exists():

        shutil.copy(bin_file, pkg_dir / f"{manifest['name']}.bin")

        binary_data = bin_file.read_bytes()

    else:

        src = app_dir / "src" / "app.c"

        shutil.copy(src, pkg_dir / "app.c")

        binary_data = src.read_bytes()



    manifest_bytes = (app_dir / "manifest.json").read_bytes()

    manifest["sha256"] = hashlib.sha256(manifest_bytes).hexdigest()

    manifest["binary_sha256"] = hashlib.sha256(binary_data).hexdigest()

    manifest["binary_size"] = len(binary_data)

    manifest["binary_path"] = f"binaries/{manifest['name']}.bin"

    manifest["signature"] = sign_bytes(binary_data)



    pkg_manifest = pkg_dir / "manifest.json"

    pkg_manifest.write_text(json.dumps(manifest, indent=2) + "\n")

    (pkg_dir / "signature.sha256").write_text(manifest["signature"] + "\n")

    print(f"Packaged {manifest['name']} v{manifest['version']}")

    print(f"  Binary SHA256: {manifest['binary_sha256']}")

    print(f"  HMAC: {manifest['signature'][:16]}...")

    print(f"  Output: {pkg_dir}")

    return pkg_dir





def main() -> int:

    if len(sys.argv) < 2:

        print("Usage: python sdk/package.py <app-dir> [target]")

        return 1

    target = sys.argv[2] if len(sys.argv) > 2 else "host"

    package(Path(sys.argv[1]), target)

    return 0





if __name__ == "__main__":

    sys.exit(main())


