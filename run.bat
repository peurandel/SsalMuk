@echo off
REM Build directory 생성 및 빌드
if not exist build (
    mkdir build
)
pushd build
cmake ..
cmake --build . -- /m
popd

REM 가능한 실행 경로들 시도
if exist build\Release\SsalMuk.exe (
    build\Release\SsalMuk.exe
) else if exist build\Debug\SsalMuk.exe (
    build\Debug\SsalMuk.exe
) else if exist build\SsalMuk.exe (
    build\SsalMuk.exe
) else (
    echo 실행파일을 찾을 수 없습니다. 빌드가 성공했는지 확인하세요.
    exit /b 1
)
