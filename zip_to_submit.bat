
@echo off
setlocal enabledelayedexpansion

REM === Configuration ===
set "BASENAME=csd2151_a8_chew.s"
set "SRC_DIR=%~dp0%BASENAME%"

REM Validate source folder exists
if not exist "%SRC_DIR%" (
    echo [ERROR] Source folder not found: "%SRC_DIR%"
    echo Check: 
    echo - You do set BASENAME parameter correctly in this bat file
    echo - You do run this bat from the parent folder where the source is located
    pause
    exit /b 1
)

echo Source folder found: "%SRC_DIR%"

REM Clean the folder
echo Cleaning the folder
cd "%SRC_DIR%"
call clean.bat 2>nul
cd ..

REM Build output ZIP path
set "ZIPNAME=%BASENAME%.zip"
set "OUT_ZIP=%~dp0%ZIPNAME%"

REM === Delete existing ZIP file in the current folder ===
echo Deleting existing ZIP file
powershell -NoProfile -Command "Remove-Item -Path '%OUT_ZIP%' -Force -ErrorAction SilentlyContinue"

echo Creating ZIP: "%OUT_ZIP%"
powershell -NoProfile -Command "Compress-Archive -LiteralPath '%SRC_DIR%' -DestinationPath '%OUT_ZIP%' -Force"

if exist "%OUT_ZIP%" (
    echo [SUCCESS] ZIP created.
    exit /b 0
) else (
    echo [ERROR] Failed to create ZIP.
    exit /b 2
)

pause

endlocal
