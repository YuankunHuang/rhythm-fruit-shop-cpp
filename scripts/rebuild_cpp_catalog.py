#!/usr/bin/env python3
"""Rebuild assets/charts/catalog.json in the C++ repo from all *.rfs.json chart files."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import import_for_cpp  # noqa: E402
from import_for_cpp import (  # noqa: E402
    catalog_entry_for,
    configure_paths,
    resolve_cpp_repo,
)

DIFF_ORDER = ["easy", "normal", "hard", "expert", "service"]


def sort_difficulties(keys: list[str]) -> list[str]:
    return sorted(keys, key=lambda d: DIFF_ORDER.index(d) if d in DIFF_ORDER else 99)


def main() -> int:
    parser = argparse.ArgumentParser(description="Rebuild C++ repo catalog.json from *.rfs.json files.")
    parser.add_argument(
        "--cpp-repo",
        type=Path,
        default=None,
        help="C++ repo root (default: this repo or RFS_CPP_REPO env)",
    )
    args = parser.parse_args()
    configure_paths(resolve_cpp_repo(args.cpp_repo))
    charts_dir = import_for_cpp.CHARTS_DIR
    catalog_path = import_for_cpp.CATALOG_PATH

    if not charts_dir.is_dir():
        print(f"Charts directory not found: {charts_dir}")
        return 1

    songs: list[dict] = []
    missing_audio: list[str] = []

    # Preserve manual fields (e.g. hidden) that the script does not manage
    manual_fields: dict[str, dict] = {}
    if catalog_path.exists():
        try:
            existing = json.loads(catalog_path.read_text(encoding="utf-8"))
            for s in existing.get("songs", []):
                sid = s.get("id", "")
                extra = {k: v for k, v in s.items()
                         if k not in ("id", "title", "audio", "chart", "cover", "difficulties")}
                if extra:
                    manual_fields[sid] = extra
        except Exception:
            pass  # corrupt or missing — ignored

    for chart_path in sorted(charts_dir.glob("*.rfs.json")):
        try:
            data = json.loads(chart_path.read_text(encoding="utf-8"))
        except Exception as exc:
            print(f"  SKIP {chart_path.name}: {exc}")
            continue

        if data.get("schema") != "rfs-cpp-v1":
            print(f"  SKIP {chart_path.name}: wrong schema")
            continue

        diffs_obj = data.get("difficulties")
        if not isinstance(diffs_obj, dict) or not diffs_obj:
            print(f"  SKIP {chart_path.name}: no difficulties")
            continue

        song_id = data.get("id") or chart_path.stem.replace(".rfs", "")
        difficulties = sort_difficulties(list(diffs_obj.keys()))

        # catalog_entry_for() fills audio + cover via filesystem probe
        entry = catalog_entry_for(song_id, difficulties)

        # Preserve the title from the chart file when it looks real
        chart_title = data.get("title", "")
        if chart_title and chart_title != song_id:
            entry["title"] = chart_title

        if not entry.get("audio"):
            missing_audio.append(song_id)

        if entry["id"] in manual_fields:
            entry.update(manual_fields[entry["id"]])

        songs.append(entry)

    catalog = {"songs": songs}
    catalog_path.write_text(
        json.dumps(catalog, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    with_audio = sum(1 for s in songs if s.get("audio"))
    print(f"Wrote {len(songs)} song(s) to {catalog_path}")
    print(f"  with audio: {with_audio}")
    print(f"  missing audio: {len(missing_audio)}")
    if missing_audio:
        for sid in missing_audio:
            print(f"    - {sid}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
