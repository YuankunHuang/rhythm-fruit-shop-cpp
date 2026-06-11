@echo off
setlocal
cd /d "%~dp0"

echo Rhythm Fruit Shop C++ - Convert source audio to MP3 192kbps
echo.
echo Default source: audio\  (override with --audio-dir)
echo Default output: assets\audio\
echo.
echo Tip: use --audio-dir ..\rhythm-fruit-shop\audio to point at the web prototype's audio folder.
echo.
echo Options:
echo   --audio-dir PATH     Source audio directory (default: audio\)
echo   --force              Re-encode even if manifest hash matches
echo   --normalize-audio    Run EBU R128 loudnorm (only if source is not pre-normalized)
echo   --dry-run            Show what would be done without converting
echo   --no-rewrite-charts  Do not update chart audio paths after conversion
echo.

python --version >nul 2>&1
if errorlevel 1 (
  echo Python was not found. Please install Python and try again.
  pause
  exit /b 1
)

ffmpeg -version >nul 2>&1
if errorlevel 1 (
  echo ffmpeg was not found. Please install ffmpeg and add it to PATH.
  pause
  exit /b 1
)

python scripts\convert_audio_for_cpp_core.py %*
if errorlevel 1 (
  echo Audio conversion failed.
  pause
  exit /b 1
)

echo.
echo Done. Runtime audio is under assets\audio\.
pause
