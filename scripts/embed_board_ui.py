#!/usr/bin/env python3
"""Embed hardware/ui/board_index.html into hermes_http.cpp INDEX_HTML."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
UI = ROOT / "hardware" / "ui" / "board_index.html"
CPP = ROOT / "hardware" / "src" / "hermes_http.cpp"
MARKER_START = 'static const char INDEX_HTML[] PROGMEM = R"HTML('
MARKER_END = ')HTML";'


def main() -> int:
    html = UI.read_text(encoding="utf-8").strip()
    cpp = CPP.read_text(encoding="utf-8")
    i0 = cpp.find(MARKER_START)
    i1 = cpp.find(MARKER_END, i0)
    if i0 < 0 or i1 < 0:
        raise SystemExit("INDEX_HTML markers not found in hermes_http.cpp")
    new_cpp = cpp[: i0 + len(MARKER_START)] + "\n" + html + "\n" + cpp[i1:]
    CPP.write_text(new_cpp, encoding="utf-8")
    print(f"embedded {len(html)} bytes from {UI}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
