@echo off
REM Build the project first, then run this script to create the Windows installer EXE.
REM Requires Inno Setup command line compiler (iscc) on PATH.

if not exist build\Release\SsalMuk.exe (
  echo build\Release\SsalMuk.exe not found. Build the project first.
  exit /b 1
)

if exist output rmdir /s /q output
mkdir output
iscc windows\SsalMukInstaller.iss
if errorlevel 1 (
  echo Installer build failed.
  exit /b 1
)
echo Installer created in output\SsalMuk-Installer.exe
