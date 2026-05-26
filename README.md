# Rhythm Fruit Shop — C++ Core

Native rhythm demo for [Rhythm Fruit Shop](../README.md). Same game world as the web prototype at the repo root, but a **separate C++ codebase** — not a port. Different charts, assets, and schema.

Four-lane falling notes. Visuals stay intentionally minimal while the clock pipeline, platform layering, and playable loop come together.

Web prototype (visuals, zh-CN narrative, shop loop) → [../README.md#web-prototype](../README.md#web-prototype)

---

## Quick Start

**Requirements:** Visual Studio 2022/2026, CMake 3.24+, vcpkg (manifest mode)

1. Open this folder (`cpp_core/`) in Visual Studio (**File → Open → Folder**).
2. Wait for CMake configure. vcpkg installs **SFML 2.6.1** and **nlohmann-json** from `vcpkg.json`.
3. Set **`rfs_demo`** as the startup project and press **F5**.

The debugger working directory is set to `cpp_core/` in CMake, so asset paths such as `assets/audio/...` resolve correctly.

If SFML DLLs are missing at runtime, copy the debug binaries from:

`out/build/x64-Debug/vcpkg_installed/x64-windows/debug/bin/`

into the same folder as `rfs_demo.exe`.

## Controls

| Key | In gameplay | In pause menu |
|-----|-------------|---------------|
| **D F J K** | Lanes 0–3 | — |
| **Enter** | Start (main menu) / confirm (loading, result) | Return to main menu |
| **Esc** | Open pause overlay | Resume |

Default window size: **1280×720**.

## Demo Flow

```
MainMenu → Loading (async chart load) → Gameplay → Result → MainMenu
                              ↑
                         Pause overlay (Esc)
```

Shipped demo chart: `assets/charts/service/lemon-water-light.json`  
Demo audio: `assets/audio/service/lemon_water_light.mp3`

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
    └── rfs_core             ChartLoader, FrozenChart, SmoothedSongClock
            └── no SFML, no miniaudio (enforced by CMake dependency guards)
```

Gameplay screens receive services through `GameContext` (`IRenderer`, `IAudioPlayer`, `UIManager`, `SmoothedSongClock`). Concrete SFML and miniaudio types never appear in `rfs_app`.

## Timing Pipeline

Song time is **not** advanced by frame `deltaTime`.

```
miniaudio PCM cursor          (IAudioBackendClock)
        ↓
SmoothedSongClock             EMA smoothing + re-anchor
        ↓
FrameContext.song_time_ms     consumed by GameplayScreen
        ↓
note spawn Y + input judgment delta vs targetTimeMs
```

Input events carry a host-monotonic timestamp at poll time; judgment compares mapped song time against each note's `timeMs`.

## CMake Targets

| Target | Role |
|--------|------|
| `rfs_core` | Rhythm core — chart load, frozen chart data, smoothed clock |
| `rfs_platform_iface` | Header-only platform contracts (`IWindow`, `IRenderer`, …) |
| `rfs_platform_sfml` | SFML 2.6 window, render, input |
| `rfs_platform_miniaudio` | Audio playback + sample-position clock |
| `rfs_app` | Screens, game loop, scoring / grading rules |
| `rfs_demo` | Runnable game executable |
| `rfs_tests` | Headless core tests (no window, no audio device) |

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
    rhythm/        ChartLoader, SmoothedSongClock, FrozenChart
  vcpkg.json       SFML 2.6.1 (pinned), nlohmann-json
```

## Engineering Notes

- **Dependency inversion:** `rfs_core` and `rfs_app` must not link SFML or miniaudio. Guards live in `cmake/DependencyGuards.cmake`.
- **UI scaling:** Reference resolution 1280×720; font sizes use uniform `min(sx, sy)` scale via `UiFontConfig`.
- **Resize:** View resets to 1:1 pixel mapping on `sf::Event::Resized`. Live reflow while dragging the window border is limited by Win32 modal resize behaviour; layout updates correctly after the mouse is released.

## License

Personal project — Yuankun Huang, 2026.
