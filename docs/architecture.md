# SsalMuk 아키텍처

## 목표
시각장애인이 Windows 환경에서 `scrcpy`로 연결된 Android 휴대폰을 LLM 기반 워크플로우로 제어할 수 있도록 지원합니다.

## 주요 구성 요소

- `src/main.cpp`
  - 사용자 명령을 입력 받음
  - LLM API URL과 모델명을 프롬프트로 받음
  - 전체 자동화 흐름을 실행함

- `src/llm_agent.*`
  - 사용자 요청을 LLM에 전달하고 응답을 받음
  - 응답에서 `tap`, `type`, `capture` 단계 추출
  - 프로바이더 추상화를 통해 Ollama 이외의 LLM도 확장 가능

- `src/ollama_provider.*`
  - Ollama API 호출 구현
  - `curl`을 사용해 HTTP POST 요청 전송

- `src/scrcpy_controller.*`
  - `scrcpy` 실행 및 Android 기기 조작
  - ADB를 사용해 탭, 키 입력, 화면 캡처 명령 실행

- `src/screen_analyzer.*`
  - 캡처된 화면 이미지를 분석하는 확장점
  - 향후 OCR/컴퓨터 비전 통합에 사용

## 워크플로우
1. 사용자 입력 또는 환경 변수로 LLM API 정보 수집
2. `scrcpy` 실행으로 Android 화면 연결
3. 사용자 명령을 LLM 프롬프트로 전송
4. LLM 응답을 계획 단계로 변환
5. ADB를 통해 화면 조작 및 캡처 수행

## 확장 포인트
- Ollama 외 `Google`, `Anthropic`, `Deepseek`, `Grok` 등의 프로바이더 연결
- 화면 OCR/객체 인식으로 좌표 자동 추출
- GUI 기반 접근성 인터페이스
- Android 앱별 시나리오 매핑 강화
