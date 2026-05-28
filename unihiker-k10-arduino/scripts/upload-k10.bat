@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: Unihiker K10 Arduino Upload Script for Windows
:: Usage: upload-k10.bat <sketch.ino> [port]

set "SKETCH=%~1"
set "PORT=%~2"
set "FQBN=UNIHIKER:esp32:k10"

if "%~1"=="" (
    echo [ERROR] No sketch file specified
    echo Usage: upload-k10.bat ^<sketch.ino^> [port]
    exit /b 1
)

if not exist "%SKETCH%" (
    echo [ERROR] Sketch file not found: %SKETCH%
    exit /b 1
)

:: Get sketch directory
set "SKETCH_DIR=%~dp1"
set "SKETCH_NAME=%~nx1"
set "BUILD_DIR=%SKETCH_DIR%build"

:: Check for arduino-cli
where arduino-cli >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] arduino-cli not found in PATH
    echo Please install arduino-cli from https://arduino.github.io/arduino-cli/latest/installation/
    exit /b 1
)

echo [INFO] Sketch: %SKETCH_NAME%
echo [INFO] FQBN: %FQBN%
echo [INFO] Build dir: %BUILD_DIR%

:: Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Detect port if not specified
if "%PORT%"=="" (
    echo [INFO] Detecting K10 port...
    for /f "tokens=*" %%a in ('arduino-cli board list ^| findstr "usb" ^| findstr /v "grep"') do (
        for /f "tokens=1" %%b in ("%%a") do (
            set "PORT=%%b"
            goto :port_found
        )
    )
)

:port_found
if "%PORT%"=="" (
    echo [ERROR] Could not detect K10 port
    echo Please specify port manually or connect K10 board
    exit /b 1
)

echo [INFO] Port: %PORT%

:: Compile
echo [INFO] Compiling...
arduino-cli compile --fqbn %FQBN% --build-path "%BUILD_DIR%" --jobs 0 "%SKETCH%"
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed
    exit /b 1
)
echo [OK] Compilation successful

:: Upload
echo [INFO] Uploading to %PORT%...
arduino-cli upload -p %PORT% --fqbn %FQBN% --input-dir "%BUILD_DIR%" "%SKETCH%"
if %errorlevel% neq 0 (
    echo [ERROR] Upload failed
    echo Tips:
    echo   - Make sure K10 is in bootloader mode ^(hold BOOT, press RST^)
    echo   - Check that the port is correct: %PORT%
    exit /b 1
)

echo [OK] Upload successful!
echo [INFO] Sketch is now running on K10

endlocal
