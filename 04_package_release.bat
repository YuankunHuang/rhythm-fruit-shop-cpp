@echo off
setlocal
cd /d "%~dp0"

echo Rhythm Fruit Shop C++ - Package Windows x64 Release
echo.
echo Step 1/2: Build Release  (win64-vcpkg preset, Release config)
echo Step 2/2: Stage dist\RhythmFruitShop-win64\  and  .zip
echo.
echo Optional: Pillow (cover optimization), ffmpeg (MP3 re-encode).
echo.

:: ── Locate MSVC via vswhere ──────────────────────────────────────────────────
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
)
if not exist "%VSWHERE%" (
  echo vswhere.exe not found. Please install Visual Studio with the C++ workload.
  pause
  exit /b 1
)

for /f "usebackq delims=" %%i in (
  `"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`
) do set "VS_PATH=%%i"

if not defined VS_PATH (
  echo Visual Studio with C++ tools not found.
  pause
  exit /b 1
)

call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 (
  echo Failed to initialize MSVC environment.
  pause
  exit /b 1
)

:: ── Python check ─────────────────────────────────────────────────────────────
python --version >nul 2>&1
if errorlevel 1 (
  echo Python was not found. Please install Python and try again.
  pause
  exit /b 1
)

:: ── Build ────────────────────────────────────────────────────────────────────
echo [1/2] Building Release...
cmake --build --preset win64-release-build --target rfs_demo
if errorlevel 1 (
  echo.
  echo Build failed. Configure the project first:
  echo   cmake --preset win64-vcpkg
  pause
  exit /b 1
)

:: ── Package ──────────────────────────────────────────────────────────────────
echo.
echo [2/2] Packaging...
python scripts\package_cpp_core_release.py %*
if errorlevel 1 (
  echo Packaging failed.
  pause
  exit /b 1
)

echo.
echo Done. Distributable folder: dist\RhythmFruitShop-win64\
echo       Zip:                  dist\RhythmFruitShop-win64.zip
echo.
pause
