@echo off
setlocal
echo =========================================================================
echo  DONT DROP THE HAMMER - DSP Pitch Shift & Strobe Tuner Guitar Plugin
echo  THE EDGE OF FEAR - VST3 System Installer / Updater
echo =========================================================================
echo.

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Administrator privileges required to install to C:\Program Files\Common Files\VST3.
    echo Requesting elevation...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process cmd -ArgumentList '/c \"\"%~f0\"\"' -Verb RunAs"
    exit /b
)

echo Cleaning previous installation from C:\Program Files\Common Files\VST3\DONT DROP THE HAMMER.vst3...
if exist "C:\Program Files\Common Files\VST3\DONT DROP THE HAMMER.vst3" (
    rd /s /q "C:\Program Files\Common Files\VST3\DONT DROP THE HAMMER.vst3"
)

echo Deploying latest DONT DROP THE HAMMER.vst3...
if exist "%~dp0DONT DROP THE HAMMER.vst3" (
    robocopy "%~dp0DONT DROP THE HAMMER.vst3" "C:\Program Files\Common Files\VST3\DONT DROP THE HAMMER.vst3" /E /IS /IT /NFL /NDL
) else if exist "%~dp0..\VST3\DONT DROP THE HAMMER.vst3" (
    robocopy "%~dp0..\VST3\DONT DROP THE HAMMER.vst3" "C:\Program Files\Common Files\VST3\DONT DROP THE HAMMER.vst3" /E /IS /IT /NFL /NDL
)

if exist "C:\Program Files\Common Files\VST3\DONT DROP THE HAMMER.vst3\Contents\x86_64-win\DONT DROP THE HAMMER.vst3" (
    echo.
    echo [SUCCESS] DONT DROP THE HAMMER.vst3 successfully installed in C:\Program Files\Common Files\VST3!
    echo Rescan plugins in your DAW (Cakewalk Sonar, Reaper, Cubase, FL Studio, Ableton, Studio One) or restart your DAW now.
) else (
    echo.
    echo [WARNING] Installation check failed. Please ensure DONT DROP THE HAMMER.vst3 is in this folder.
)

echo.
pause
