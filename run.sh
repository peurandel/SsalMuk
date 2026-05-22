#!/usr/bin/env bash
set -e

echo "Building SsalMuk..."
mkdir -p build
cd build
cmake ..
cmake --build . -- -j$(nproc)

echo "Running SsalMuk..."
EXEC=./SsalMuk
if [ ! -x "$EXEC" ]; then
  echo "실행파일 $EXEC을(를) 찾을 수 없습니다. 빌드가 성공했는지 확인하세요."
  exit 1
fi

$EXEC
