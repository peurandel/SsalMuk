@echo off
setlocal enabledelayedexpansion

echo ====================================================
echo SsalMuk Windows 설치 준비 도구
echo ====================================================
echo.
set "DOWNLOAD_DIR=%~dp0deps"
if not exist "%DOWNLOAD_DIR%" mkdir "%DOWNLOAD_DIR%"
echo 다운로드 폴더: %DOWNLOAD_DIR%
echo.

set "HAS_CMAKE=1"
set "HAS_ADB=1"
set "HAS_SCRCPY=1"
set "HAS_ISCC=1"

where cmake >nul 2>&1 || set "HAS_CMAKE=0"
where adb >nul 2>&1 || set "HAS_ADB=0"
where scrcpy >nul 2>&1 || set "HAS_SCRCPY=0"
where iscc >nul 2>&1 || set "HAS_ISCC=0"

echo 검사 결과:
if "%HAS_CMAKE%"=="1" (echo - CMake: 설치됨) else (echo - CMake: 없음)
if "%HAS_ADB%"=="1" (echo - adb: 설치됨) else (echo - adb: 없음)
if "%HAS_SCRCPY%"=="1" (echo - scrcpy: 설치됨) else (echo - scrcpy: 없음)
if "%HAS_ISCC%"=="1" (echo - Inno Setup iscc: 설치됨) else (echo - Inno Setup iscc: 없음)
echo.

set "POWERSHELL_CMD=powershell -NoProfile -InputFormat None -ExecutionPolicy Bypass"

:download_cmake
if "%HAS_CMAKE%"=="1" goto skip_cmake
echo CMake가 필요합니다.
set "CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v3.39.2/cmake-3.39.2-windows-x86_64.msi"
set "CMAKE_FILE=%DOWNLOAD_DIR%\cmake-3.39.2-windows-x86_64.msi"
if not exist "%CMAKE_FILE%" (
    echo CMake 설치 프로그램을 다운로드합니다...
    %POWERSHELL_CMD% "Invoke-WebRequest -Uri '%CMAKE_URL%' -OutFile '%CMAKE_FILE%'"
)
echo CMake 설치 파일 준비 완료: %CMAKE_FILE%
echo 이 파일을 실행하여 CMake를 설치하세요.
echo.
goto skip_cmake
:skip_cmake

:download_adb
if "%HAS_ADB%"=="1" goto skip_adb
echo adb가 필요합니다.
set "ADB_URL=https://dl.google.com/android/repository/platform-tools-latest-windows.zip"
set "ADB_FILE=%DOWNLOAD_DIR%\platform-tools-latest-windows.zip"
set "ADB_DIR=%DOWNLOAD_DIR%\platform-tools"
if not exist "%ADB_FILE%" (
    echo Android platform-tools를 다운로드합니다...
    %POWERSHELL_CMD% "Invoke-WebRequest -Uri '%ADB_URL%' -OutFile '%ADB_FILE%'"
)
if not exist "%ADB_DIR%" (
    echo 압축을 풉니다...
    %POWERSHELL_CMD% "Expand-Archive -Force '%ADB_FILE%' '%ADB_DIR%'"
)
echo adb 설치 파일 준비 완료: %ADB_DIR%
echo adb 경로를 PATH에 추가하세요: %ADB_DIR%
echo.
goto skip_adb
:skip_adb

:download_scrcpy
if "%HAS_SCRCPY%"=="1" goto skip_scrcpy
echo scrcpy가 필요합니다.
set "SCRCPY_URL=https://github.com/Genymobile/scrcpy/releases/download/v1.27/scrcpy-win64-v1.27.zip"
set "SCRCPY_FILE=%DOWNLOAD_DIR%\scrcpy-win64-v1.27.zip"
set "SCRCPY_DIR=%DOWNLOAD_DIR%\scrcpy-win64"
if not exist "%SCRCPY_FILE%" (
    echo scrcpy를 다운로드합니다...
    %POWERSHELL_CMD% "Invoke-WebRequest -Uri '%SCRCPY_URL%' -OutFile '%SCRCPY_FILE%'"
)
if not exist "%SCRCPY_DIR%" (
    echo 압축을 풉니다...
    %POWERSHELL_CMD% "Expand-Archive -Force '%SCRCPY_FILE%' '%SCRCPY_DIR%'"
)
echo scrcpy 설치 파일 준비 완료: %SCRCPY_DIR%
echo scrcpy.exe 경로를 PATH에 추가하세요: %SCRCPY_DIR%\scrcpy-win64
echo.
goto skip_scrcpy
:skip_scrcpy

:download_iscc
if "%HAS_ISCC%"=="1" goto skip_iscc
echo Inno Setup iscc가 없습니다. 설치 프로그램을 만들려면 필요합니다.
echo Inno Setup은 https://jrsoftware.org 에서 설치할 수 있습니다.
echo.
goto skip_iscc
:skip_iscc

:final
echo ====================================================
echo 준비가 완료되었습니다.
echo - CMake가 설치되었는지 확인하세요.
echo - adb와 scrcpy가 설치 또는 PATH에 포함되어 있어야 합니다.
echo - 필요한 항목이 없으면 위 메시지를 따라 설치하세요.
echo.
echo deps 폴더도 확인하세요: %DOWNLOAD_DIR%
echo.
pause
endlocal
