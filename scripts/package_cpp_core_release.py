#!/usr/bin/env python3
"""Stage a shareable Windows x64 release folder and optional zip."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BUILD_ROOT = ROOT / "out" / "build" / "win64-vcpkg"
DEFAULT_OUT = ROOT / "dist" / "RhythmFruitShop-win64"
DEFAULT_ZIP = ROOT / "dist" / "RhythmFruitShop-win64.zip"
DEFAULT_ASSETS_SRC = ROOT / "assets"
SHARE_SCRIPT = ROOT / "scripts" / "package_cpp_core_share.py"

PLAY_TXT = """Rhythm Fruit Shop - Windows x64

1. Unzip this folder anywhere.
2. Double-click RhythmFruitShop.exe
3. Keep assets\\ next to the exe.

Controls:
  D F J K     - lanes
  Up/Down     - song select
  Left/Right  - difficulty
  Enter       - confirm
  Esc         - back / pause

If the game fails to start, install Microsoft Visual C++ Redistributable x64.
"""


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Stage Rhythm Fruit Shop Windows x64 release.")
    p.add_argument(
        "--build-root",
        type=Path,
        default=DEFAULT_BUILD_ROOT,
        help="CMake build directory (Ninja Multi-Config preset output)",
    )
    p.add_argument(
        "--out",
        type=Path,
        default=DEFAULT_OUT,
        help="Staged release folder",
    )
    p.add_argument(
        "--zip",
        type=Path,
        default=DEFAULT_ZIP,
        help="Output zip path (use --no-zip to skip)",
    )
    p.add_argument("--no-zip", action="store_true", help="Do not create a zip archive")
    p.add_argument(
        "--assets-source",
        type=Path,
        default=DEFAULT_ASSETS_SRC,
        help="Source assets directory",
    )
    p.add_argument(
        "--configuration",
        default="Release",
        help="CMake configuration folder name (default: Release)",
    )
    return p.parse_args()


def copy_runtime_dlls(vcpkg_bin: Path, out_dir: Path) -> int:
    if not vcpkg_bin.is_dir():
        raise FileNotFoundError(f"vcpkg bin folder not found: {vcpkg_bin}")
    dlls = sorted(vcpkg_bin.glob("*.dll"))
    if not dlls:
        raise FileNotFoundError(f"No DLLs found in {vcpkg_bin}")
    for dll in dlls:
        shutil.copy2(dll, out_dir / dll.name)
    return len(dlls)


def stage_assets(assets_source: Path, out_assets: Path) -> None:
    if not assets_source.is_dir():
        raise FileNotFoundError(f"Assets folder not found: {assets_source}")

    if SHARE_SCRIPT.is_file():
        subprocess.run(
            [
                sys.executable,
                str(SHARE_SCRIPT),
                "--target",
                str(out_assets),
                "--source",
                str(assets_source),
            ],
            check=True,
            cwd=ROOT,
        )
        return

    # Fallback if share script is missing: copy runtime dirs only (not showcase/).
    runtime_dirs = ("audio", "charts", "covers", "fonts")
    if out_assets.exists():
        shutil.rmtree(out_assets)
    out_assets.mkdir(parents=True)
    for name in runtime_dirs:
        src = assets_source / name
        if src.is_dir():
            shutil.copytree(src, out_assets / name)


def create_zip(out_dir: Path, zip_path: Path) -> None:
    zip_path.parent.mkdir(parents=True, exist_ok=True)
    if zip_path.exists():
        zip_path.unlink()
    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        for path in sorted(out_dir.rglob("*")):
            if path.is_file():
                zf.write(path, path.relative_to(out_dir).as_posix())


def main() -> int:
    args = parse_args()
    build_root = args.build_root.resolve()
    exe = build_root / args.configuration / "RhythmFruitShop.exe"
    vcpkg_bin = build_root / "vcpkg_installed" / "x64-windows" / "bin"
    out_dir = args.out.resolve()

    if not exe.is_file():
        print(f"[ERROR] Expected exe not found: {exe}", file=sys.stderr)
        return 1

    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    shutil.copy2(exe, out_dir / exe.name)
    dll_count = copy_runtime_dlls(vcpkg_bin, out_dir)
    print(f"Copied exe and {dll_count} DLL(s) to {out_dir}")

    stage_assets(args.assets_source.resolve(), out_dir / "assets")
    (out_dir / "PLAY.txt").write_text(PLAY_TXT, encoding="utf-8")

    if not args.no_zip:
        zip_path = args.zip.resolve()
        create_zip(out_dir, zip_path)
        print(f"Created zip: {zip_path}")

    print(f"Release folder: {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
