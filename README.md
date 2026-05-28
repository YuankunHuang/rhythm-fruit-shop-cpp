# Rhythm Fruit Shop — C++ Core

Native rhythm demo for [Rhythm Fruit Shop](../README.md). Same game world as the web prototype at the repo root, but a **separate C++ codebase** — not a port. Different charts, assets, and schema.

Four-lane falling notes. Visuals stay intentionally minimal while the clock pipeline, platform layering, and playable loop come together.

Web prototype (visuals, zh-CN narrative, shop loop) → [../README.md#web-prototype](../README.md#web-prototype)

---

## Quick Start

**Requirements:** Visual Studio 2022/2026 with **Desktop development with C++**, CMake 3.24+, vcpkg (manifest mode)

1. Open this folder (`cpp_core/`) in Visual Studio (**File → Open → Folder**).
2. Select CMake preset **`win64-vcpkg`** (Ninja Multi-Config + vcpkg manifest).
3. Wait for configure. vcpkg installs **SFML 2.6.1** and **nlohmann-json** from `vcpkg.json`.
4. Set **`rfs_demo`** as the startup project and press **F5**.

The debugger working directory is set to `cpp_core/` in CMake, so asset paths such as `assets/audio/...` resolve correctly.

If `VCPKG_ROOT` is not set globally, Visual Studio usually picks up the vcpkg bundled with the C++ workload. From a command prompt you can set:

```bat
set VCPKG_ROOT=C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg
```

If SFML DLLs are missing at runtime during local Debug builds, copy the debug binaries from:

`out/build/win64-vcpkg/Debug/vcpkg_installed/x64-windows/debug/bin/`

into the same folder as `RhythmFruitShop.exe` (or use the Release packaging flow below).

## Controls

| Key | In gameplay | In pause menu | In song select |
|-----|-------------|---------------|----------------|
| **D F J K** | Lanes 0–3 | — | — |
| **Up / Down** | — | — | Change song |
| **Left / Right** | — | — | Change difficulty |
| **Enter** | Start (main menu) / confirm (loading, result) | Return to main menu | Start selected song |
| **Esc** | Open pause overlay | Resume | Back |

Default window size: **1280×720**.

## Demo Flow

```
MainMenu → ChartSelect → Loading (async chart load) → Gameplay → Result → MainMenu
                              ↑
                         Pause overlay (Esc)
```

Song list and audio paths come from `assets/charts/catalog.json`. Rebuild the catalog after importing charts or converting audio:

```bat
python ..\scripts\rebuild_cpp_catalog.py
```

Import osu!mania charts from `imports/<song-id>/mug/*.osz`:

```bat
..\03_import_for_cpp.bat
```

Example playable entries:

| Song ID | Difficulty | Audio |
|---------|------------|-------|
| `lemon-water-light` | service | `assets/audio/service/lemon_water_light.mp3` |
| `lets-drive` | easy | `assets/audio/tracks/lets_drive.mp3` |
| `drama` | easy/normal/hard/expert | `assets/audio/tracks/drama.mp3` |

## Architecture

```
main.cpp / rfs_demo          composition root — wires concrete backends
    │
    ├── rfs_app              Application, UIManager, IScreen implementations
    │       └── depends on platform interfaces only (no SFML / miniaudio)
    │
    ├── rfs_platform_sfml    SfmlWindow, SfmlRenderer, SfmlInputSource
    ├── rfs_platform_miniaudio   MiniaudioAudioPlayer, MiniaudioAudioBackendClock
    │
    └── rfs_core             ChartLoader, FrozenChart, SmoothedSongClock, AudioPathResolver
            └── no SFML, no miniaudio (enforced by CMake dependency guards)
```

Gameplay screens receive services through `GameContext` (`IRenderer`, `IAudioPlayer`, `UIManager`, `SmoothedSongClock`). Concrete SFML and miniaudio types never appear in `rfs_app`.

## Timing Pipeline

Song time is **not** advanced by frame `deltaTime`.

```
miniaudio PCM cursor          (IAudioBackendClock)
        ↓
SmoothedSongClock             host interpolation + EMA re-anchor + pause freeze
        ↓
FrameContext.song_time_ms     consumed by GameplayScreen
        ↓
note spawn Y + input judgment (event_song_time_ms from reverse-mapped host timestamp)
```

Input events carry a per-key host-monotonic timestamp at poll time; judgment compares mapped song time against each note's `timeMs`.

## CMake Targets

| Target | Role |
|--------|------|
| `rfs_core` | Rhythm core — chart load, frozen chart data, smoothed clock, audio path resolver |
| `rfs_platform_iface` | Header-only platform contracts (`IWindow`, `IRenderer`, …) |
| `rfs_platform_sfml` | SFML 2.6 window, render, input |
| `rfs_platform_miniaudio` | Audio playback + sample-position clock |
| `rfs_app` | Screens, game loop, scoring / grading rules |
| `rfs_demo` | Runnable game executable |
| `rfs_tests` | Headless core tests (no window, no audio device) |

Run tests from the `cpp_core/` working directory:

```bat
out\build\win64-vcpkg\Debug\rfs_tests.exe
```

## Release Packaging (Windows x64)

From the **repository root** (not `cpp_core/`):

```bat
08_package_cpp_core_release.bat
```

This script:

1. Uses Visual Studio's bundled CMake (not an older PATH cmake)
2. Configures preset **`win64-vcpkg`** and builds **`win64-release-build`**
3. Stages `dist\RhythmFruitShop-win64\` (exe, SFML DLLs, optimized assets, `PLAY.txt`)
4. Creates `dist\RhythmFruitShop-win64.zip`

Requirements: Visual Studio with C++ workload, Python 3 (+ Pillow for asset optimization). Optional: `ffmpeg` on PATH for mp3 re-encode during asset staging.

The same staging logic is available as:

```bat
python scripts\package_cpp_core_release.py --build-root cpp_core\out\build\win64-vcpkg
```

### CI

GitHub Actions workflow **C++ Release (Windows x64)** (`.github/workflows/cpp-release-windows.yml`) can be triggered manually via **workflow_dispatch**. It builds with the same CMake presets and uploads `RhythmFruitShop-win64.zip` as an artifact.

## Project Layout

```
cpp_core/
  assets/          charts, audio, fonts
  cmake/           warnings, dependency guards
  external/        miniaudio (single-header)
  src/
    app/           Application, UIManager, IScreen
    game/          screens, layout, rules
    platform/      interfaces + SFML / miniaudio backends
    rhythm/        ChartLoader, SmoothedSongClock, FrozenChart, AudioPathResolver
  vcpkg.json       SFML 2.6.1 (pinned), nlohmann-json
```

## Engineering Notes

- **Dependency inversion:** `rfs_core` and `rfs_app` must not link SFML or miniaudio. Guards live in `cmake/DependencyGuards.cmake`.
- **UI scaling:** Reference resolution 1280×720; font sizes use uniform `min(sx, sy)` scale via `UiFontConfig`.
- **Resize:** View resets to 1:1 pixel mapping on `sf::Event::Resized`. Live reflow while dragging the window border is limited by Win32 modal resize behaviour; layout updates correctly after the mouse is released.

## License

Personal project — Yuankun Huang, 2026.
