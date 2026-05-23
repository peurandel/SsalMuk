| id | task | status | notes |
| --- | --- | --- | --- |
| 1 | 프로젝트 초기 C++ 스켈레톤 생성 | done | CMake, main, LLM/스크립트/분석 모듈 추가 |
| 2 | scrcpy/ADB 제어 모듈 구현 | done | `scrcpy_controller`에서 launch/tap/type/capture 기능 구현 |
| 3 | LLM 요청 분석 및 계획 생성 | done | `llm_agent`에서 Ollama API 호출 및 JSON 파싱 구현 |
| 4 | 화면 캡처 및 OCR 분석 통합 | done | `screen_analyzer`에 tesseract CLI 기반 OCR 프로토타입 추가 |
| 5 | Gmail 및 게임 시나리오 샘플 추가 | todo | 실제 명령 매핑 로직 개발 |
| 6 | LLM 응답 파싱 및 ADB 입력 안정화 | done | `main.cpp`, `llm_agent.cpp`, `scrcpy_controller.cpp`, `ollama_provider.cpp` 수정 |
| 7 | Vulkan 기반 클라이언트 UI 프로토타입 | done | GLFW/Vulkan로 기본 윈도우와 렌더 루프 생성 |
