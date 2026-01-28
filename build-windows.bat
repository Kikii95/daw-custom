@echo off
REM Build script for Windows - DAW Custom
REM Requires: Visual Studio 2022 with C++ workload

echo ========================================
echo DAW Custom - Windows Build
echo ========================================

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake not found. Install CMake and add to PATH.
    exit /b 1
)

REM Configure with Visual Studio generator
echo.
echo [1/2] Configuring CMake...
cmake -B build -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

REM Build Release
echo.
echo [2/2] Building Release...
cmake --build build --config Release --parallel

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESS!
echo ========================================
echo.
echo Executable: build\DAWCustom_artefacts\Release\DAW Custom.exe
echo.
pause
