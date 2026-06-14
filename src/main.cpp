#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "action_script.h"
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

static bool getEnvBool(const char* envName) {
    const char* envValue = std::getenv(envName);
    if (!envValue) {
        return false;
    }
    std::string value(envValue);
    for (auto& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value == "1" || value == "true" || value == "yes" || value == "on";
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
        std::cerr << "scrcpy 실행에 실패했습니다. 사유: " << controller.lastError() << std::endl;
        return 1;
    }

    LlmAgent llm(apiUrl, model);
    ScreenAnalyzer analyzer;
    bool dryRun = getEnvBool("SSALMUK_DRY_RUN");

    const std::vector<std::pair<std::string, std::string>> sampleScripts = {
        {"gmail", "scripts/gmail_sample.txt"},
        {"game", "scripts/game_sample.txt"}
    };

    if (dryRun) {
        std::cout << "[DRY-RUN] 명령을 실제로 실행하지 않습니다. 환경 변수 SSALMUK_DRY_RUN=1을 사용 중입니다." << std::endl;
    }

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
        if (userInput == "list-samples") {
            std::cout << "사용 가능한 샘플 스크립트:" << std::endl;
            for (const auto& sample : sampleScripts) {
                std::cout << "  " << sample.first << " -> " << sample.second << std::endl;
            }
            continue;
        }
        if (userInput.rfind("run-sample ", 0) == 0 || userInput.rfind("run-script ", 0) == 0) {
            std::string path;
            if (userInput.rfind("run-sample ", 0) == 0) {
                std::string sampleName = userInput.substr(std::string("run-sample ").size());
                bool found = false;
                for (const auto& sample : sampleScripts) {
                    if (sample.first == sampleName) {
                        path = sample.second;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cerr << "알 수 없는 샘플 이름입니다: " << sampleName << std::endl;
                    continue;
                }
            } else {
                path = userInput.substr(std::string("run-script ").size());
            }

            std::ifstream inputFile(path);
            if (!inputFile) {
                std::cerr << "스크립트 파일을 열 수 없습니다: " << path << std::endl;
                continue;
            }

            std::string scriptSource((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
            ActionScript script;
            if (!script.parse(scriptSource)) {
                std::cerr << "스크립트 파싱 실패: " << script.lastError() << std::endl;
                continue;
            }

            std::cout << "스크립트 명령 수: " << script.commands().size() << std::endl;
            for (const auto& command : script.commands()) {
                std::cout << "  " << command.action;
                if (command.action == "tap") {
                    std::cout << " " << command.x.value_or(-1) << " " << command.y.value_or(-1);
                } else if (command.action == "type") {
                    std::cout << " " << command.text;
                } else if (command.action == "wait") {
                    std::cout << " " << command.waitMs << "ms";
                }
                std::cout << std::endl;
            }

            if (!dryRun) {
                if (!script.execute(controller, &analyzer, dryRun)) {
                    std::cerr << "스크립트 실행 실패: " << script.lastError() << std::endl;
                    continue;
                }
            }
            std::cout << "스크립트 실행을 마쳤습니다." << std::endl;
            continue;
        }

        auto plan = llm.analyzeUserRequest(userInput);
        if (!plan) {
            std::cerr << "LLM이 명령을 이해하지 못했습니다." << std::endl;
            continue;
        }

        std::cout << "LLM 계획: " << plan->description << std::endl;
        ActionScript planScript;
        if (!planScript.loadFromPlan(*plan)) {
            std::cerr << "LLM 계획을 스크립트로 변환하지 못했습니다: " << planScript.lastError() << std::endl;
            continue;
        }

        for (const auto& command : planScript.commands()) {
            std::cout << "실행: " << command.action;
            if (command.action == "tap") {
                std::cout << " " << command.x.value_or(-1) << " " << command.y.value_or(-1);
            } else if (command.action == "type") {
                std::cout << " " << command.text;
            } else if (command.action == "wait") {
                std::cout << " " << command.waitMs << "ms";
            }
            std::cout << std::endl;
        }

        if (!dryRun) {
            if (!planScript.execute(controller, &analyzer, dryRun)) {
                std::cerr << "LLM 계획 실행 실패: " << planScript.lastError() << std::endl;
                continue;
            }
        }

        std::cout << "작업 완료를 시도했습니다. 상태를 확인하세요." << std::endl;
    }

    return 0;
}
