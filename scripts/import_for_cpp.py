#!/usr/bin/env python3
"""
Import osu!mania .osz drafts from imports/ into the C++ project chart format.

Expected import layout:
    imports/<song-id>/mug/<difficulty>.osz   (e.g. easy / normal / hard / expert)

Output (default: sibling rhythm-fruit-shop-cpp repo):
    assets/charts/<song-id>.rfs.json   -- rfs-cpp-v1 chart
    assets/charts/catalog.json         -- updated song catalog

Notes:
  - The rfs-cpp-v1 format uses time_ms (integer milliseconds) and lanes 0-3.
  - 4K osu!mania maps to lanes 0-3 directly (column N -> lane N).
  - Other key counts are scaled to 4 lanes.
  - Audio is NOT handled by this script. Place the audio file at:
        assets/audio/<song-id>.mp3  (or .m4a / .ogg)
    in the C++ repo before running.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import sys
import zipfile
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent
DEFAULT_CPP_REPO = ROOT.parent / "rhythm-fruit-shop-cpp"


def resolve_cpp_repo(path: Path | None = None) -> Path:
    if path is not None:
        return path if path.is_absolute() else ROOT / path
    env = os.environ.get("RFS_CPP_REPO")
    if env:
        return Path(env)
    return DEFAULT_CPP_REPO


CPP_REPO = resolve_cpp_repo()
CHARTS_DIR = CPP_REPO / "assets" / "charts"
AUDIO_DIR = CPP_REPO / "assets" / "audio"
IMPORTS_DIR = ROOT / "imports"
CATALOG_PATH = CHARTS_DIR / "catalog.json"

COVERS_DIR = CPP_REPO / "assets" / "covers"

VALID_DIFFICULTIES = {"easy", "normal", "hard", "expert", "service"}
AUDIO_EXTENSIONS = (".mp3", ".m4a", ".ogg", ".wav")


# ---------------------------------------------------------------------------
# .osu parsing helpers
# ---------------------------------------------------------------------------

def sections_from_text(text: str) -> dict[str, list[str]]:
    sections: dict[str, list[str]] = {}
    current: str | None = None
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("//"):
            continue
        if line.startswith("[") and line.endswith("]"):
            current = line[1:-1]
            sections.setdefault(current, [])
            continue
        if current:
            sections[current].append(line)
    return sections


def read_sections_from_osz(osz_path: Path) -> tuple[dict[str, list[str]], str]:
    with zipfile.ZipFile(osz_path) as archive:
        osu_names = sorted(name for name in archive.namelist() if name.lower().endswith(".osu"))
        if not osu_names:
            raise RuntimeError(f"{osz_path.name}: no .osu file inside archive")
        osu_name = osu_names[0]
        text = archive.read(osu_name).decode("utf-8-sig", errors="replace")
        return sections_from_text(text), osu_name


def key_values(lines: list[str]) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in lines:
        if ":" not in line:
            continue
        key, value = line.split(":", 1)
        result[key.strip()] = value.strip()
    return result


def parse_int(value: str, default: int = 0) -> int:
    try:
        return int(float(value))
    except (ValueError, TypeError):
        return default


# ---------------------------------------------------------------------------
# Lane mapping
# ---------------------------------------------------------------------------

def lane_for_column(column: int, key_count: int) -> int:
    """Map an osu!mania column to a C++ 4-lane index (0-3)."""
    if key_count <= 0:
        return 0
    if key_count == 4:
        return max(0, min(3, column))
    # Scale other key counts proportionally to 4 lanes
    normalized = column / max(1, key_count - 1)
    return min(3, int(normalized * 4))


def column_for_x(x: int, key_count: int) -> int:
    return max(0, min(key_count - 1, int(math.floor(x * key_count / 512))))


# ---------------------------------------------------------------------------
# Note parsing
# ---------------------------------------------------------------------------

def parse_hit_objects(lines: list[str], key_count: int) -> list[dict]:
    notes: list[dict] = []
    for line in lines:
        parts = line.split(",")
        if len(parts) < 5:
            continue
        x = parse_int(parts[0])
        time_ms = parse_int(parts[2])
        object_type = parse_int(parts[3])
        column = column_for_x(x, key_count)
        lane = lane_for_column(column, key_count)

        notes.append({
            "id": len(notes),
            "time_ms": time_ms,
            "lane": lane,
            "visual": len(notes) % 4,
        })
    return notes


# ---------------------------------------------------------------------------
# Catalog management
# ---------------------------------------------------------------------------

def load_catalog() -> dict:
    if CATALOG_PATH.exists():
        try:
            return json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
        except Exception:
            pass
    return {"songs": []}


def save_catalog(catalog: dict) -> None:
    CATALOG_PATH.write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def humanize_song_id(song_id: str) -> str:
    return song_id.replace("-", " ").replace("_", " ").title()


def find_audio(song_id: str) -> str | None:
    """Return a relative audio path (assets/audio/...) if a file exists, else None.
    Searches root, tracks/, and service/ subdirectories."""
    subdirs = ["", "tracks", "service"]
    for sub in subdirs:
        for ext in AUDIO_EXTENSIONS:
            candidate = AUDIO_DIR / sub / (song_id + ext) if sub else AUDIO_DIR / (song_id + ext)
            if candidate.exists():
                return candidate.relative_to(CPP_REPO).as_posix()
    return None


def find_cover(song_id: str) -> str | None:
    """Return a relative cover path (assets/covers/<song_id>/cover.png) if the file exists."""
    candidate = COVERS_DIR / song_id / "cover.png"
    if candidate.exists():
        return candidate.relative_to(CPP_REPO).as_posix()
    return None


def catalog_entry_for(song_id: str, difficulties: list[str]) -> dict:
    audio = find_audio(song_id)
    cover = find_cover(song_id)
    return {
        "id": song_id,
        "title": humanize_song_id(song_id),
        "audio": audio or "",
        "chart": f"assets/charts/{song_id}.rfs.json",
        "cover": cover or "",
        "difficulties": difficulties
    }


def update_catalog(song_id: str, difficulties: list[str]) -> None:
    catalog = load_catalog()
    songs: list[dict] = catalog.setdefault("songs", [])

    existing = next((s for s in songs if s.get("id") == song_id), None)
    entry = catalog_entry_for(song_id, difficulties)

    if existing is None:
        songs.append(entry)
    else:
        existing.update(entry)

    save_catalog(catalog)


# ---------------------------------------------------------------------------
# Per-song import
# ---------------------------------------------------------------------------

def import_song(song_dir: Path, overwrite: bool) -> bool:
    song_id = song_dir.name
    mug_dir = song_dir / "mug"
    if not mug_dir.is_dir():
        print(f"  Skipping {song_id}: no mug/ subdirectory")
        return False

    osz_files = sorted(mug_dir.glob("*.osz"))
    if not osz_files:
        print(f"  Skipping {song_id}: no .osz files in mug/")
        return False

    chart_path = CHARTS_DIR / f"{song_id}.rfs.json"

    if chart_path.exists() and not overwrite:
        existing = json.loads(chart_path.read_text(encoding="utf-8"))
    else:
        existing = {
            "schema": "rfs-cpp-v1",
            "id": song_id,
            "title": song_id,
            "approach_time_ms": 1600,
            "difficulties": {},
        }

    title = humanize_song_id(song_id)
    imported_diffs: list[str] = []

    for osz_path in osz_files:
        diff_name = osz_path.stem.lower()
        if diff_name not in VALID_DIFFICULTIES:
            print(f"    Unknown difficulty '{diff_name}', skipping {osz_path.name}")
            continue

        try:
            sections, osu_name = read_sections_from_osz(osz_path)
        except Exception as e:
            print(f"    ERROR reading {osz_path.name}: {e}")
            continue

        general = key_values(sections.get("General", []))
        metadata = key_values(sections.get("Metadata", []))
        difficulty = key_values(sections.get("Difficulty", []))

        mode = parse_int(general.get("Mode", "3"), 3)
        if mode != 3:
            print(f"    Skipping {osz_path.name}: not an osu!mania map (Mode={mode})")
            continue

        key_count = parse_int(difficulty.get("CircleSize", "4"), 4)
        if key_count != 4:
            print(f"    Warning: {osz_path.name} has {key_count} keys; folding to 4 lanes")

        notes = parse_hit_objects(sections.get("HitObjects", []), key_count)

        existing["difficulties"][diff_name] = notes
        imported_diffs.append(diff_name)

        print(f"    {diff_name}: {len(notes)} notes  ({osu_name})")

    if not imported_diffs:
        return False

    existing["title"] = title
    existing["id"] = song_id

    # Sort difficulties in standard order
    order = ["easy", "normal", "hard", "expert", "service"]
    all_diffs = sorted(existing["difficulties"].keys(), key=lambda d: order.index(d) if d in order else 99)

    chart_path.write_text(
        json.dumps(existing, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8"
    )

    audio = find_audio(song_id)
    if not audio:
        print(f"    Warning: no audio found at {AUDIO_DIR / song_id}.<ext>")

    update_catalog(song_id, all_diffs)
    print(f"  Wrote {chart_path.relative_to(ROOT)}")
    return True


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def refresh_catalog() -> None:
    """Rebuild catalog.json from existing .rfs.json files without re-parsing any .osz."""
    rfs_files = sorted(CHARTS_DIR.glob("*.rfs.json"))
    if not rfs_files:
        print("No .rfs.json files found in charts dir.")
        return

    # Start fresh
    catalog: dict = {"songs": []}

    for rfs_path in rfs_files:
        try:
            data = json.loads(rfs_path.read_text(encoding="utf-8-sig"))
        except Exception as e:
            print(f"  Skip {rfs_path.name}: {e}")
            continue

        song_id = data.get("id", rfs_path.stem.replace(".rfs", ""))
        order = ["easy", "normal", "hard", "expert", "service"]
        diffs = sorted(data.get("difficulties", {}).keys(),
                       key=lambda d: order.index(d) if d in order else 99)
        if not diffs:
            print(f"  Skip {rfs_path.name}: no difficulties")
            continue

        entry = catalog_entry_for(song_id, diffs)
        # Preserve title from the chart file if it looks real
        chart_title = data.get("title", "")
        if chart_title and chart_title != song_id:
            entry["title"] = chart_title
        catalog["songs"].append(entry)
        cover_status = "ok" if entry["cover"] else "no cover"
        audio_status = "ok" if entry["audio"] else "no audio"
        print(f"  {song_id}: {len(diffs)} diff(s), audio={audio_status}, cover={cover_status}")

    save_catalog(catalog)
    print()
    print(f"Catalog refreshed: {len(catalog['songs'])} song(s).")
    print(f"Catalog: {CATALOG_PATH}")


def configure_paths(cpp_repo: Path) -> None:
    global CPP_REPO, CHARTS_DIR, AUDIO_DIR, CATALOG_PATH, COVERS_DIR
    CPP_REPO = cpp_repo.resolve()
    CHARTS_DIR = CPP_REPO / "assets" / "charts"
    AUDIO_DIR = CPP_REPO / "assets" / "audio"
    CATALOG_PATH = CHARTS_DIR / "catalog.json"
    COVERS_DIR = CPP_REPO / "assets" / "covers"


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Import osu!mania .osz files from imports/ into the C++ chart format."
    )
    parser.add_argument(
        "--cpp-repo",
        type=Path,
        default=None,
        help=f"C++ repo root (default: sibling {DEFAULT_CPP_REPO.name}/ or RFS_CPP_REPO env)",
    )
    parser.add_argument(
        "--song", metavar="SONG_ID",
        help="Import only this song (subdirectory name under imports/). Imports all if omitted."
    )
    parser.add_argument(
        "--overwrite", action="store_true",
        help="Overwrite existing difficulties in the chart file. Default: merge (keep existing)."
    )
    parser.add_argument(
        "--refresh-catalog", action="store_true",
        help="Rebuild catalog.json from existing .rfs.json files without re-parsing any .osz."
    )
    args = parser.parse_args()

    configure_paths(resolve_cpp_repo(args.cpp_repo))
    print(f"C++ repo: {CPP_REPO}")
    CHARTS_DIR.mkdir(parents=True, exist_ok=True)

    if args.refresh_catalog:
        refresh_catalog()
        return

    if args.song:
        dirs = [IMPORTS_DIR / args.song]
    else:
        dirs = sorted(p for p in IMPORTS_DIR.iterdir() if p.is_dir())

    if not dirs:
        print("No song directories found in imports/.")
        sys.exit(0)

    success = 0
    for song_dir in dirs:
        if not song_dir.is_dir():
            print(f"Not found: {song_dir}")
            continue
        print(f"{song_dir.name}")
        if import_song(song_dir, args.overwrite):
            success += 1

    print()
    print(f"Imported {success} / {len(dirs)} song(s).")
    print(f"Catalog: {CATALOG_PATH}")


if __name__ == "__main__":
    main()
