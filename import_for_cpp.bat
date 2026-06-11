@echo off
setlocal
cd /d "%~dp0"

echo Rhythm Fruit Shop C++ - Import osu!mania charts
echo.
echo Reads imports\^<song-id^>\mug\^<difficulty^>.osz and writes to assets\charts\.
echo.
echo Options:
echo   --song SONG_ID       Import only one song
echo   --overwrite          Re-parse and overwrite existing chart data
echo   --refresh-catalog    Rebuild catalog.json from existing .rfs.json (no .osz needed)
echo.

python --version >nul 2>&1
if errorlevel 1 (
  echo Python was not found. Please install Python and try again.
  pause
  exit /b 1
)

python scripts\import_for_cpp.py %*
if errorlevel 1 (
  echo Import failed.
  pause
  exit /b 1
)

echo.
echo Done. Rebuild rfs_demo to pick up catalog changes.
pause
