#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

struct LlmStep {
    std::string action;
    std::optional<int> x;
    std::optional<int> y;
    std::string text;
};

struct LlmPlan {
    std::string description;
    std::vector<LlmStep> steps;
};

class LlmAgent {
public:
    LlmAgent(const std::string& apiUrl, const std::string& model);
    ~LlmAgent();

    std::optional<LlmPlan> analyzeUserRequest(const std::string& request);
    // 테스트 및 외부 사용을 위한 응답 파서(LLM 응답 텍스트 -> LlmPlan)
    static std::optional<LlmPlan> parseResponseToPlan(const std::string& response);

private:
    std::unique_ptr<class LlmProvider> provider_;
    static std::string buildPrompt(const std::string& request);
    static std::optional<LlmPlan> parseProviderResponse(const std::string& response);
};
