#include <cstdlib>
#include <iostream>
#include <string>
#include "llm_agent.h"
#include "scrcpy_controller.h"
#include "screen_analyzer.h"

static std::string getEnvOrPrompt(const char* envName, const char* prompt, const std::string& defaultValue) {
    const char* envValue = std::getenv(envName);
    if (envValue && envValue[0] != '\0') {
        return std::string(envValue);
    }

    std::cout << prompt;
    if (!defaultValue.empty()) {
        std::cout << " (기본: " << defaultValue << ")";
    }
    std::cout << ": ";
    std::string value;
    std::getline(std::cin, value);
    if (value.empty()) {
        return defaultValue;
    }
    return value;
}

int main() {
    std::cout << "SsalMuk: 시각장애인용 Android 자동화 도우미" << std::endl;
    std::cout << "Windows + scrcpy + Ollama 기반 API 테스트 프로토타입입니다.\n";

    const std::string apiUrl = getEnvOrPrompt("OLLAMA_API_URL", "LLM API URL을 입력하세요", "https://ollama.com/v1/chat/completions");
    const std::string model = getEnvOrPrompt("OLLAMA_MODEL", "모델명을 입력하세요", "gpt-4o");

    std::cout << "LLM 엔드포인트: " << apiUrl << "\n";
    std::cout << "모델: " << model << "\n";
    std::cout << "API 키가 필요한 경우 OLLAMA_API_KEY 환경 변수를 설정하세요.\n";

    ScrcpyController controller;
    if (!controller.launch()) {
        std::cerr << "scrcpy 실행에 실패했습니다. scrcpy가 설치되었는지 확인하세요." << std::endl;
        return 1;
    }

    LlmAgent llm(apiUrl, model);
    ScreenAnalyzer analyzer;

    while (true) {
        std::cout << "\n명령 입력 (종료하려면 quit 입력): ";
        std::string userInput;
        std::getline(std::cin, userInput);
        if (userInput.empty()) {
            continue;
        }
        if (userInput == "quit") {
            break;
        }

        auto plan = llm.analyzeUserRequest(userInput);
        if (!plan) {
            std::cerr << "LLM이 명령을 이해하지 못했습니다." << std::endl;
            continue;
        }

        std::cout << "LLM 계획: " << plan->description << std::endl;
        for (const auto& step : plan->steps) {
            std::cout << "실행: " << step << std::endl;
            if (step.rfind("tap", 0) == 0) {
                int x, y;
                if (sscanf(step.c_str(), "tap %d %d", &x, &y) == 2) {
                    controller.tap(x, y);
                }
            } else if (step.rfind("type", 0) == 0) {
                std::string text = step.substr(5);
                controller.typeText(text);
            } else if (step == "capture") {
                controller.captureScreen();
                analyzer.analyzeScreen();
            }
        }

        std::cout << "작업 완료를 시도했습니다. 상태를 확인하세요." << std::endl;
    }

    return 0;
}
