@echo off
REM Build directory 생성 및 빌드
if not exist build (
    mkdir build
)
pushd build
cmake ..
if errorlevel 1 (
    echo CMake 구성에 실패했습니다.
    popd
    pause
    exit /b 1
)
cmake --build . -- /m
if errorlevel 1 (
    echo 빌드에 실패했습니다.
    popd
    pause
    exit /b 1
)
popd

REM 가능한 실행 경로들 시도
if exist build\Release\SsalMuk.exe (
    call build\Release\SsalMuk.exe
) else if exist build\Debug\SsalMuk.exe (
    call build\Debug\SsalMuk.exe
) else if exist build\SsalMuk.exe (
    call build\SsalMuk.exe
) else (
    echo 실행파일을 찾을 수 없습니다. 빌드가 성공했는지 확인하세요.
    pause
    exit /b 1
)

echo.
echo 프로그램이 종료되었습니다. 오류 로그를 확인하세요.
pause
