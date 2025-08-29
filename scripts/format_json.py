#!/usr/bin/env python3
"""
format_json.py
Format all JSON files in the Survival Project repository.

This script recursively formats JSON files under `data/json` and
`data/mods` directories using `json.dump` with indentation.

Usage:
  python3 scripts/format_json.py

Note: This tool rewrites files in-place. Commit your changes after
running this script to ensure consistent formatting.
"""
import json
from pathlib import Path


def format_directory(path: Path) -> None:
    for file_path in path.rglob("*.json"):
        with file_path.open("r", encoding="utf-8") as f:
            data = json.load(f)
        with file_path.open("w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
            f.write("\n")


def main() -> None:
    project_root = Path(__file__).resolve().parents[1]
    data_dirs = [project_root / "data" / "json", project_root / "data" / "mods"]
    for d in data_dirs:
        if d.exists():
            format_directory(d)


if __name__ == "__main__":
    main()
