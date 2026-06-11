#!/usr/bin/env python3
"""Export miniaudio-friendly MP3 into the C++ repo from a source audio directory.

Source audio is expected to be already loudness-normalized (e.g. run through
00_convert_audio_to_m4a.bat in the web prototype project first).

Default source: <repo-root>/audio/  (override with --audio-dir)
Default output: <repo-root>/assets/audio/
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

DEFAULT_CPP_REPO = ROOT.parent / "rhythm-fruit-shop-cpp"
DEFAULT_AUDIO_DIR = ROOT / "audio"
DEFAULT_OUT_DIR = ROOT / "assets" / "audio"
CPP_REPO = ROOT
MANIFEST_NAME = "cpp-audio-manifest.json"
BITRATE_KBPS = 192
SAMPLE_RATE = 48000
CHANNELS = 2
ENCODER_SETTINGS = {
    "codec": "mp3",
    "bitrate_kbps": BITRATE_KBPS,
    "sample_rate": SAMPLE_RATE,
    "channels": CHANNELS,
}

# ---------------------------------------------------------------------------
# Inlined utilities (from convert_audio_to_m4a.py in the web prototype)
# ---------------------------------------------------------------------------

RUNTIME_AUDIO_SUFFIXES = {".opus", ".mp3", ".ogg", ".wav", ".flac", ".m4a"}
LOUDNESS_MANIFEST = "loudness-manifest.json"
LOUDNORM_TARGET = {"i": -16.0, "tp": -1.5, "lra": 11.0}

try:
    sys.stdout.reconfigure(encoding="utf-8")
except AttributeError:
    pass


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def audio_files(audio_dir: Path, include_m4a: bool = True) -> list[Path]:
    if not audio_dir.exists():
        return []
    suffixes = RUNTIME_AUDIO_SUFFIXES if include_m4a else (RUNTIME_AUDIO_SUFFIXES - {".m4a"})
    return sorted(path for path in audio_dir.rglob("*") if path.is_file() and path.suffix.lower() in suffixes)


def ffmpeg_loudnorm_filter(measured: dict[str, str] | None = None) -> str:
    parts = [
        f"I={LOUDNORM_TARGET['i']}",
        f"TP={LOUDNORM_TARGET['tp']}",
        f"LRA={LOUDNORM_TARGET['lra']}",
    ]
    if measured:
        parts.extend([
            f"measured_I={measured['input_i']}",
            f"measured_TP={measured['input_tp']}",
            f"measured_LRA={measured['input_lra']}",
            f"measured_thresh={measured['input_thresh']}",
            f"offset={measured['target_offset']}",
            "linear=true",
        ])
    parts.append("print_format=json")
    return "loudnorm=" + ":".join(parts)


def parse_loudnorm_json(stderr: str) -> dict[str, str]:
    matches = list(re.finditer(r"\{[\s\S]*?\}", stderr))
    for match in reversed(matches):
        try:
            data = json.loads(match.group(0))
        except json.JSONDecodeError:
            continue
        if "input_i" in data or "output_i" in data:
            return data
    raise RuntimeError("ffmpeg loudnorm did not produce JSON stats")


def analyze_loudness(src: Path, ffmpeg: str) -> dict[str, str]:
    result = subprocess.run(
        [ffmpeg, "-hide_banner", "-nostats", "-i", str(src),
         "-map", "0:a:0", "-af", ffmpeg_loudnorm_filter(), "-f", "null", "-"],
        capture_output=True, text=True, encoding="utf-8", errors="replace", check=True,
    )
    return parse_loudnorm_json(result.stderr)


# ---------------------------------------------------------------------------
# Core conversion logic
# ---------------------------------------------------------------------------

def rel(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return str(path)


def chart_audio_path(out_file: Path) -> str:
    return out_file.relative_to(CPP_REPO).as_posix()


def audio_sources(audio_dir: Path) -> list[Path]:
    return [
        path
        for path in audio_files(audio_dir, include_m4a=True)
        if path.name != LOUDNESS_MANIFEST
    ]


def collect_jobs(audio_dir: Path, out_dir: Path) -> list[tuple[Path, Path]]:
    return [(src, output_path_for_source(audio_dir, out_dir, src)) for src in audio_sources(audio_dir)]


def load_manifest(path: Path) -> dict[str, object]:
    if not path.exists():
        return {"version": 1, "encoder": ENCODER_SETTINGS, "files": {}}
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
        if not isinstance(data.get("files"), dict):
            data["files"] = {}
        data["version"] = data.get("version", 1)
        data["encoder"] = ENCODER_SETTINGS
        return data
    except Exception:
        return {"version": 1, "encoder": ENCODER_SETTINGS, "files": {}}


def write_manifest(path: Path, records: list[dict[str, object]]) -> None:
    if not records:
        return
    data = load_manifest(path)
    files = data.setdefault("files", {})
    for record in records:
        files[str(record["output"])] = record
    data["updated_at"] = datetime.now(timezone.utc).isoformat()
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"  wrote {rel(path)}")


def record_matches(record: dict | None, src: Path, dst: Path) -> bool:
    if not record:
        return False
    if record.get("encoder") != ENCODER_SETTINGS:
        return False
    if not dst.exists():
        return False
    if record.get("source_sha256") != sha256_file(src):
        return False
    if record.get("output_sha256") != sha256_file(dst):
        return False
    return True


def output_path_for_source(audio_dir: Path, out_dir: Path, src: Path) -> Path:
    rel_src = src.relative_to(audio_dir)
    return out_dir / rel_src.with_suffix(".mp3")


def convert_one(src: Path, dst: Path, ffmpeg: str, normalize_audio: bool) -> dict[str, object]:
    dst.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(suffix=".mp3", dir=dst.parent, delete=False) as tmp:
        tmp_path = Path(tmp.name)

    analysis: dict[str, str] | None = None
    output_stats: dict[str, str] | None = None
    source_hash = sha256_file(src)
    try:
        cmd = [ffmpeg, "-y", "-v", "error",
               "-i", str(src), "-vn", "-map", "0:a:0", "-map_metadata", "-1"]
        if normalize_audio:
            analysis = analyze_loudness(src, ffmpeg)
            cmd.extend(["-af", ffmpeg_loudnorm_filter(analysis)])
        cmd.extend([
            "-ac", str(CHANNELS),
            "-ar", str(SAMPLE_RATE),
            "-c:a", "libmp3lame",
            "-b:a", f"{BITRATE_KBPS}k",
            "-f", "mp3",
            str(tmp_path),
        ])
        result = subprocess.run(
            cmd,
            capture_output=normalize_audio,
            text=normalize_audio,
            encoding="utf-8" if normalize_audio else None,
            errors="replace" if normalize_audio else None,
            check=True,
        )
        if normalize_audio and result.stderr:
            output_stats = parse_loudnorm_json(result.stderr)

        tmp_path.replace(dst)
        suffix = " [loudnorm]" if normalize_audio else ""
        print(f"  -> {rel(dst)}{suffix}", flush=True)
        return {
            "source": rel(src),
            "output": rel(dst),
            "chart_audio": chart_audio_path(dst),
            "source_sha256": source_hash,
            "output_sha256": sha256_file(dst),
            "encoder": ENCODER_SETTINGS,
            "analysis": analysis,
            "result": output_stats,
            "normalized": normalize_audio,
            "updated_at": datetime.now(timezone.utc).isoformat(),
        }
    except Exception:
        tmp_path.unlink(missing_ok=True)
        raise


def build_chart_replacements(src: Path, dst: Path) -> dict[str, str]:
    replacements: dict[str, str] = {}
    chart_path = chart_audio_path(dst)
    src_rel = rel(src)
    replacements[src_rel] = chart_path
    replacements[f"assets/audio/{src.stem}.mp3"] = chart_path
    replacements[f"assets/audio/{src.name}"] = chart_path

    out_root = CPP_REPO / "assets" / "audio"
    try:
        rel_under_out = dst.relative_to(out_root)
    except ValueError:
        rel_under_out = Path(dst.name)

    for suffix in sorted(RUNTIME_AUDIO_SUFFIXES):
        replacements[(Path("assets/audio") / rel_under_out.with_suffix(suffix)).as_posix()] = chart_path
    replacements[(Path("assets/audio") / rel_under_out.with_suffix(".mp3")).as_posix()] = chart_path

    return replacements


def rewrite_cpp_charts(replacements: dict[str, str]) -> None:
    if not replacements:
        return
    charts_dir = CPP_REPO / "assets" / "charts"
    if not charts_dir.exists():
        return
    for path in sorted(charts_dir.rglob("*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        audio = data.get("audio")
        if not isinstance(audio, str):
            continue
        new_audio = replacements.get(audio)
        if not new_audio or new_audio == audio:
            continue
        data["audio"] = new_audio
        path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(f"  rewrote chart audio {rel(path)}: {audio} -> {new_audio}")


def resolve_cpp_repo(path: Path | None = None) -> Path:
    import os
    if path is not None:
        return path if path.is_absolute() else ROOT / path
    env = os.environ.get("RFS_CPP_REPO")
    if env:
        return Path(env)
    return DEFAULT_CPP_REPO


def configure_cpp_repo(cpp_repo: Path) -> None:
    global CPP_REPO, DEFAULT_OUT_DIR
    CPP_REPO = cpp_repo.resolve()
    DEFAULT_OUT_DIR = CPP_REPO / "assets" / "audio"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Transcode audio to MP3 192kbps for the C++ repo.\n"
                    "Sources are assumed already loudness-normalized.\n"
                    "Default --audio-dir is <repo>/audio/; override to point at the web prototype's audio/ folder."
    )
    parser.add_argument(
        "--cpp-repo", type=Path, default=None,
        help=f"C++ repo root (default: this repo or RFS_CPP_REPO env)",
    )
    parser.add_argument("--audio-dir", type=Path, default=DEFAULT_AUDIO_DIR,
                        help="Directory containing source audio files")
    parser.add_argument("--out-dir", type=Path, default=None)
    parser.add_argument("--ffmpeg", default="ffmpeg")
    parser.add_argument("--force", action="store_true", help="Re-encode even when manifest hash matches")
    parser.add_argument("--normalize-audio", action="store_true",
                        help="Run EBU R128 loudnorm (only if source was not pre-normalized)")
    parser.add_argument("--no-rewrite-charts", action="store_true",
                        help="Do not update assets/charts/*.json audio paths")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    configure_cpp_repo(resolve_cpp_repo(args.cpp_repo))
    print(f"C++ repo: {CPP_REPO}")

    audio_dir = args.audio_dir if args.audio_dir.is_absolute() else ROOT / args.audio_dir
    out_dir = args.out_dir if args.out_dir else DEFAULT_OUT_DIR
    out_dir = out_dir if out_dir.is_absolute() else ROOT / out_dir
    normalize = args.normalize_audio
    manifest_path = out_dir / MANIFEST_NAME
    manifest = load_manifest(manifest_path)
    manifest_files = manifest.get("files", {}) if isinstance(manifest.get("files"), dict) else {}

    jobs = collect_jobs(audio_dir, out_dir)
    print(f"Conversion jobs: {len(jobs)} (sources under {rel(audio_dir)})")
    if not jobs:
        print("No source audio found.")
        print(f"Place files under {audio_dir} or use --audio-dir to point at the web prototype's audio/ folder.")
        return 1

    chart_replacements: dict[str, str] = {}
    records: list[dict[str, object]] = []
    processed = skipped = 0

    total = len(jobs)
    for i, (src, dst) in enumerate(jobs, 1):
        out_key = rel(dst)
        already_done = not args.force and record_matches(manifest_files.get(out_key), src, dst)
        if args.dry_run:
            verb = "would skip" if already_done else "would convert"
            print(f"  {verb} {rel(src)} -> {out_key}")
            skipped += already_done
            processed += not already_done
            chart_replacements.update(build_chart_replacements(src, dst))
            continue

        if already_done:
            print(f"[{i}/{total}] skip {out_key} (manifest match)")
            skipped += 1
            chart_replacements.update(build_chart_replacements(src, dst))
            continue

        print(f"[{i}/{total}] {rel(src)}", flush=True)
        record = convert_one(src, dst, args.ffmpeg, normalize)
        records.append(record)
        chart_replacements.update(build_chart_replacements(src, dst))
        processed += 1

    if args.dry_run:
        print(f"Dry run: would_process={processed} would_skip={skipped} total={len(jobs)}")
        return 0

    write_manifest(manifest_path, records)
    if not args.no_rewrite_charts:
        rewrite_cpp_charts(chart_replacements)
    print(f"Done. processed={processed} skipped={skipped} total={len(jobs)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        print(f"ffmpeg failed with exit code {exc.returncode}", file=sys.stderr)
        raise SystemExit(exc.returncode) from exc
    except Exception as exc:
        print(f"Error: {exc}", file=sys.stderr)
        raise SystemExit(1) from exc
