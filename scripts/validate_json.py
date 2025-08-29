#!/usr/bin/env python3
"""
validate_json.py
Simple JSON validator script for the Survival Project.

This script recursively searches for `.json` files under the
`data/json` and `data/mods` directories and attempts to parse each file.
If any file fails to parse, an error is printed and the script exits
with a non-zero status code.

Usage:
  python3 scripts/validate_json.py

This script is intended to be run as part of CI to ensure that
JSON data remains syntactically valid.
"""
import json
import sys
from pathlib import Path


def validate_directory(path: Path) -> bool:
    """Attempt to parse all JSON files under a directory."""
    ok = True
    for file_path in path.rglob("*.json"):
        try:
            with file_path.open("r", encoding="utf-8") as f:
                json.load(f)
        except Exception as e:
            print(f"Error in {file_path}: {e}")
            ok = False
    return ok


def main() -> int:
    project_root = Path(__file__).resolve().parents[1]
    data_dirs = [project_root / "data" / "json", project_root / "data" / "mods"]
    all_ok = True
    for d in data_dirs:
        if d.exists():
            if not validate_directory(d):
                all_ok = False
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
