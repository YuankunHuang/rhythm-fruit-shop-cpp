#!/usr/bin/env python3
"""Stage cpp_core assets for a shareable Windows build (optimized copy)."""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

MAX_WORKERS = min(8, (os.cpu_count() or 4))

try:
    from PIL import Image

    HAS_PILLOW = True
except ImportError:
    HAS_PILLOW = False

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = ROOT / "assets"

# Runtime-only top-level dirs. Repo also keeps assets/showcase/ for README /
# portfolio review media (GIF/MP4); that must not ship in playable builds.
RUNTIME_ASSET_DIRS = frozenset({"audio", "charts", "covers", "fonts"})

AUDIO_SUFFIXES = {".mp3", ".ogg", ".wav", ".flac", ".m4a"}

# Full-screen cover backgrounds use DrawCoverFill (scale-to-cover). Target at least 1080p
# so 1920x1080 windows are not upscaled from a tiny source. Web share uses 800x450
# because those covers are mostly thumbnails on mobile.
COVER_MAX_SIZE = (1920, 1080)
COVER_JPEG_QUALITY = 90
MP3_TARGET_KBPS = 128


def has_ffmpeg() -> bool:
    try:
        subprocess.run(["ffmpeg", "-version"], capture_output=True, check=True)
        return True
    except (FileNotFoundError, subprocess.CalledProcessError):
        return False


HAS_FFMPEG = has_ffmpeg()


def rel(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return str(path)


def kb(path: Path) -> float:
    return path.stat().st_size / 1024


def copy_file(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def minify_json(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    try:
        data = json.loads(src.read_text(encoding="utf-8"))
        dst.write_text(json.dumps(data, ensure_ascii=False, separators=(",", ":")), encoding="utf-8")
        print(f"  json {rel(src)}  {kb(src):.0f}KB -> {kb(dst):.0f}KB")
    except Exception:
        copy_file(src, dst)
        print(f"  copied {rel(src)}")


def reencode_mp3(src: Path, dst: Path, kbps: int) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(suffix=".mp3", delete=False, dir=dst.parent) as tmp:
        tmp_path = Path(tmp.name)
    try:
        subprocess.run(
            [
                "ffmpeg",
                "-y",
                "-i",
                str(src),
                "-vn",
                "-map",
                "0:a:0",
                "-c:a",
                "libmp3lame",
                "-b:a",
                f"{kbps}k",
                str(tmp_path),
            ],
            capture_output=True,
            check=True,
        )
        shutil.move(str(tmp_path), str(dst))
        print(f"  mp3  {rel(src)}  {kb(src):.0f}KB -> {kb(dst):.0f}KB  ({kbps}kbps)")
    except subprocess.CalledProcessError as exc:
        tmp_path.unlink(missing_ok=True)
        err = exc.stderr.decode(errors="replace")[:200]
        print(f"  ffmpeg failed for {rel(src)}: {err}")
        copy_file(src, dst)
        print(f"  fallback: copied {rel(src)}")


def optimize_cover_jpeg(src: Path, dst: Path, cover_max: tuple[int, int], jpeg_quality: int) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    img = Image.open(src)
    if img.mode not in ("RGB", "L"):
        img = img.convert("RGB")
    img.thumbnail(cover_max, Image.LANCZOS)
    jpg_dst = dst.with_suffix(".jpg")
    img.save(jpg_dst, "JPEG", quality=jpeg_quality, optimize=True, progressive=True)
    print(f"  jpg  {rel(src)}  {kb(src):.0f}KB -> {kb(jpg_dst):.0f}KB  ({cover_max[0]}x{cover_max[1]} max)")


def optimize_fallback_png(src: Path, dst: Path, cover_max: tuple[int, int]) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False, dir=dst.parent) as tmp:
        tmp_path = Path(tmp.name)
    try:
        img = Image.open(src)
        if img.mode not in ("RGBA", "RGB", "P", "L"):
            img = img.convert("RGBA")
        img.thumbnail(cover_max, Image.LANCZOS)
        img.save(tmp_path, "PNG", optimize=True, compress_level=9)
        if kb(tmp_path) < kb(src):
            shutil.move(str(tmp_path), str(dst))
            print(f"  png  {rel(src)}  {kb(src):.0f}KB -> {kb(dst):.0f}KB  (fallback)")
        else:
            tmp_path.unlink(missing_ok=True)
            copy_file(src, dst)
            print(f"  png  {rel(src)}  kept original ({kb(src):.0f}KB)")
    except Exception:
        tmp_path.unlink(missing_ok=True)
        copy_file(src, dst)
        print(f"  copied {rel(src)}  (fallback)")


def patch_catalog_cover_paths(catalog_path: Path, cover_map: dict[str, str]) -> None:
    if not cover_map or not catalog_path.exists():
        return
    data = json.loads(catalog_path.read_text(encoding="utf-8"))
    changed = 0
    for song in data.get("songs", []):
        cover = song.get("cover")
        if cover in cover_map:
            song["cover"] = cover_map[cover]
            changed += 1
    catalog_path.write_text(json.dumps(data, ensure_ascii=False, separators=(",", ":")), encoding="utf-8")
    print(f"  patched catalog cover paths: {changed}")


def is_runtime_asset(rel_path: Path) -> bool:
    """True for files under the runtime asset roots (not showcase/docs media)."""
    return bool(rel_path.parts) and rel_path.parts[0] in RUNTIME_ASSET_DIRS


def iter_runtime_files(source: Path):
    for path in sorted(source.rglob("*")):
        if not path.is_file():
            continue
        rel_path = path.relative_to(source)
        if is_runtime_asset(rel_path):
            yield path, rel_path


def copy_tree_plain(source: Path, target: Path) -> int:
    count = 0
    for path, rel_path in iter_runtime_files(source):
        copy_file(path, target / rel_path)
        count += 1
    return count


def stage_assets(
    source: Path,
    target: Path,
    *,
    optimize: bool,
    audio_kbps: int,
    cover_max: tuple[int, int],
    cover_quality: int,
) -> dict[str, float]:
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True)

    skipped_roots = sorted(
        {p.name for p in source.iterdir() if p.is_dir() and p.name not in RUNTIME_ASSET_DIRS}
    )
    if skipped_roots:
        print(f"Skipping non-runtime asset dirs: {', '.join(skipped_roots)}")

    if not optimize:
        count = copy_tree_plain(source, target)
        total_mb = sum(f.stat().st_size for f in target.rglob("*") if f.is_file()) / 1024 / 1024
        print(f"Copied {count} files without optimization.")
        print(f"Assets size: {total_mb:.1f} MB")
        return {"files": count, "mb": total_mb}

    cover_map: dict[str, str] = {}
    audio_jobs: list[tuple[Path, Path]] = []
    chart_jobs: list[tuple[Path, Path]] = []
    copied = 0

    for path, rel_path in iter_runtime_files(source):
        dst = target / rel_path
        suffix = path.suffix.lower()

        if suffix == ".json" and rel_path.parts[0] == "charts":
            chart_jobs.append((path, dst))
            continue

        if suffix in AUDIO_SUFFIXES and rel_path.parts[0] == "audio":
            audio_jobs.append((path, dst))
            continue

        if suffix == ".png" and rel_path.parts[0] == "covers":
            if path.name == "cover-fallback.png":
                if HAS_PILLOW:
                    optimize_fallback_png(path, dst, cover_max)
                else:
                    copy_file(path, dst)
                copied += 1
                continue

            if path.name == "cover.png" and HAS_PILLOW:
                optimize_cover_jpeg(path, dst, cover_max, cover_quality)
                old = f"assets/covers/{rel_path.parent.name}/cover.png"
                new = f"assets/covers/{rel_path.parent.name}/cover.jpg"
                cover_map[old] = new
                copied += 1
                continue

        copy_file(path, dst)
        copied += 1

    with ThreadPoolExecutor(max_workers=MAX_WORKERS) as pool:
        chart_futures = [pool.submit(minify_json, src, dst) for src, dst in chart_jobs]
        if HAS_FFMPEG:
            audio_futures = [pool.submit(reencode_mp3, src, dst, audio_kbps) for src, dst in audio_jobs]
        else:
            audio_futures = [pool.submit(copy_file, src, dst) for src, dst in audio_jobs]

        for future in as_completed(chart_futures + audio_futures):
            future.result()

    patch_catalog_cover_paths(target / "charts" / "catalog.json", cover_map)

    total_mb = sum(f.stat().st_size for f in target.rglob("*") if f.is_file()) / 1024 / 1024
    print("")
    print(f"Pillow: {'enabled' if HAS_PILLOW else 'disabled (install Pillow for cover resize)'}")
    print(f"ffmpeg: {'enabled' if HAS_FFMPEG else 'disabled (install ffmpeg for mp3 re-encode)'}")
    print(f"Assets size: {total_mb:.1f} MB")
    return {"files": copied + len(chart_jobs) + len(audio_jobs), "mb": total_mb}


def main() -> None:
    parser = argparse.ArgumentParser(description="Stage optimized cpp_core assets for Windows share builds.")
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--target", type=Path, required=True)
    parser.add_argument("--no-optimize", action="store_true", help="Plain copy; skip audio/cover/json optimization.")
    parser.add_argument("--audio-kbps", type=int, default=MP3_TARGET_KBPS)
    parser.add_argument("--cover-max", type=int, nargs=2, metavar=("W", "H"), default=COVER_MAX_SIZE)
    parser.add_argument("--cover-quality", type=int, default=COVER_JPEG_QUALITY, help="JPEG quality 1-95.")
    args = parser.parse_args()

    if not (1 <= args.cover_quality <= 95):
        raise SystemExit("--cover-quality must be between 1 and 95.")

    cover_max = tuple(args.cover_max)
    source = args.source if args.source.is_absolute() else ROOT / args.source
    target = args.target if args.target.is_absolute() else ROOT / args.target

    if not source.exists():
        raise SystemExit(f"Missing source assets: {source}")

    print(f"Source: {source}")
    print(f"Target: {target}")
    print(f"Optimize: {'no' if args.no_optimize else 'yes'}")
    if not args.no_optimize:
        print(f"Covers: max {cover_max[0]}x{cover_max[1]}, JPEG q{args.cover_quality}")
    print("")

    stage_assets(
        source,
        target,
        optimize=not args.no_optimize,
        audio_kbps=args.audio_kbps,
        cover_max=cover_max,
        cover_quality=args.cover_quality,
    )


if __name__ == "__main__":
    main()
