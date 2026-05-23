#include "llm_agent.h"
#include <iostream>

int main() {
    // 샘플 LLM 응답 (JSON 포함)
    std::string sampleResponse = R"(
자연어 설명:
설정 앱을 열고 네트워크 설정에 진입하여 WiFi를 켜겠습니다.

{"description": "WiFi 활성화", "steps": [
  {"action": "tap", "x": 100, "y": 200},
  {"action": "tap", "x": 150, "y": 250},
  {"action": "type", "text": "MyWiFi"},
  {"action": "capture"}
]}
    )";

    std::cout << "=== JSON 파싱 테스트 ===" << std::endl;
    std::cout << "샘플 응답:\n" << sampleResponse << std::endl;
    std::cout << "\n파싱 결과:" << std::endl;

    auto plan = LlmAgent::parseResponseToPlan(sampleResponse);
    if (plan) {
        std::cout << "✓ 파싱 성공!" << std::endl;
        std::cout << "  설명: " << plan->description << std::endl;
        std::cout << "  단계 개수: " << plan->steps.size() << std::endl;
        for (size_t i = 0; i < plan->steps.size(); ++i) {
            const auto& step = plan->steps[i];
            std::cout << "    [" << (i + 1) << "] action=" << step.action;
            if (step.x) std::cout << " x=" << *step.x;
            if (step.y) std::cout << " y=" << *step.y;
            if (!step.text.empty()) std::cout << " text=\"" << step.text << "\"";
            std::cout << std::endl;
        }
    } else {
        std::cout << "✗ 파싱 실패 - 원인 분석:" << std::endl;
        std::cout << "  입력 길이: " << sampleResponse.length() << std::endl;
        
        // 수동 디버깅
        if (sampleResponse.find("{") == std::string::npos) {
            std::cout << "  -> JSON 블록을 찾을 수 없음" << std::endl;
        } else {
            std::cout << "  -> JSON 블록 발견 위치: " << sampleResponse.find("{") << std::endl;
        }
    }

    // 추가 테스트: legacy 형식
    std::string legacyResponse = R"(
다음과 같이 진행합니다:
tap 100 200
type Hello
capture
    )";

    std::cout << "\n=== Legacy 형식 테스트 ===" << std::endl;
    std::cout << "샘플 응답:\n" << legacyResponse << std::endl;

    auto legacyPlan = LlmAgent::parseResponseToPlan(legacyResponse);
    if (legacyPlan) {
        std::cout << "✓ Legacy 파싱 성공!" << std::endl;
        std::cout << "  단계 개수: " << legacyPlan->steps.size() << std::endl;
    } else {
        std::cout << "✗ Legacy 파싱 실패" << std::endl;
    }

    return 0;
}
