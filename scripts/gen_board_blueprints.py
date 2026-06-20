#!/usr/bin/env python3
"""Generate PROGMEM blueprint catalog for ESP8266 on-board App Studio."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BLUEPRINT_DIR = ROOT / "blueprints"
OUT_H = ROOT / "hardware" / "src" / "hermes_blueprints_data.h"

ID_W = 24
NAME_W = 32


def esc(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')


def pad(s: str, width: int) -> str:
    if len(s) >= width:
        return s[: width - 1]
    return s


def main() -> int:
    items = []
    for path in sorted(BLUEPRINT_DIR.glob("*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        items.append({
            "id": data["id"],
            "name": data["name"],
            "description": data.get("description", ""),
        })

    list_json = json.dumps({"blueprints": items}, separators=(",", ":"))

    lines = [
        "#ifndef HERMES_BLUEPRINTS_DATA_H",
        "#define HERMES_BLUEPRINTS_DATA_H",
        "",
        "#include <stddef.h>",
        "#include <string.h>",
        "",
        f"#define HERMES_BOARD_BLUEPRINT_COUNT {len(items)}",
        "",
        "static const char HERMES_BLUEPRINTS_JSON[] PROGMEM =",
    ]
    chunk = 80
    for i in range(0, len(list_json), chunk):
        part = esc(list_json[i : i + chunk])
        lines.append(f'    "{part}"')
    lines[-1] += ";"
    lines += [
        "",
        f"static const char HERMES_BP_IDS[{len(items)}][{ID_W}] PROGMEM = {{",
    ]
    for item in items:
        lines.append(f'    "{pad(item["id"], ID_W)}",')
    lines += [
        "};",
        "",
        f"static const char HERMES_BP_NAMES[{len(items)}][{NAME_W}] PROGMEM = {{",
    ]
    for item in items:
        lines.append(f'    "{pad(esc(item["name"]), NAME_W)}",')
    lines += [
        "};",
        "",
        "static inline int hermes_board_blueprint_index(const char *id) {",
        "    if (!id || !id[0]) return -1;",
        f"    for (size_t i = 0; i < {len(items)}; i++) {{",
        f"        char buf[{ID_W}];",
        "        strncpy_P(buf, HERMES_BP_IDS[i], sizeof(buf) - 1);",
        "        buf[sizeof(buf) - 1] = '\\0';",
        "        if (strcmp(buf, id) == 0) return (int)i;",
        "    }",
        "    return -1;",
        "}",
        "",
        "#endif",
        "",
    ]
    OUT_H.write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {OUT_H} ({len(items)} blueprints, {len(list_json)} bytes json)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
