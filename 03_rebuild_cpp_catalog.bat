@echo off
setlocal
cd /d "%~dp0"

echo Rhythm Fruit Shop C++ - Rebuild catalog.json from existing .rfs.json files
echo.
echo Scans assets\charts\*.rfs.json and regenerates catalog.json.
echo Use this after manually editing chart files or adding new songs.
echo.

python --version >nul 2>&1
if errorlevel 1 (
  echo Python was not found. Please install Python and try again.
  pause
  exit /b 1
)

python scripts\rebuild_cpp_catalog.py %*
if errorlevel 1 (
  echo Catalog rebuild failed.
  pause
  exit /b 1
)

echo.
echo Done. Rebuild rfs_demo to pick up catalog changes.
pause
