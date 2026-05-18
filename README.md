# SsalMuk

Windows 환경에서 `scrcpy`로 연결된 Android 기기를 C++ 기반 LLM 자동화 워크플로우로 제어하기 위한 초기 프로젝트입니다.

## 개요
- 시각장애인을 위한 Android 조작 보조 도구
- `scrcpy`로 Android 화면을 Windows에서 연동
- Ollama 기반 LLM API를 통해 사용자 명령을 분석
- ADB로 화면 탭, 텍스트 입력, 캡처를 수행
- 우선 `콘솔 기반 프로토타입`으로 설계

## LLM 설정
- `OLLAMA_API_URL` 환경 변수 또는 런타임 프롬프트로 API 엔드포인트 입력
- `OLLAMA_MODEL` 환경 변수 또는 런타임 프롬프트로 모델명 입력
- `OLLAMA_API_KEY` 환경 변수로 Ollama Cloud 인증 토큰을 설정 가능
- 기본값: `https://ollama.com/v1/chat/completions`

## 폴더 구조
- `src/`: 핵심 C++ 소스 코드
- `docs/architecture.md`: 시스템 아키텍처 설명
- `CMakeLists.txt`: 빌드 구성

## 빌드
1. Windows에서 CMake 설치
2. `build` 디렉토리 생성
3. `cmake ..`
4. `cmake --build .`

## 다음 단계
- Ollama 외 다른 LLM 제공자 확장
- OCR/컴퓨터 비전 통합
- 앱 탐색/조작 시나리오 구체화
