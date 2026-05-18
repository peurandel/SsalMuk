#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

struct LlmPlan {
    std::string description;
    std::vector<std::string> steps;
};

class LlmAgent {
public:
    LlmAgent(const std::string& apiUrl, const std::string& model);
    ~LlmAgent();

    std::optional<LlmPlan> analyzeUserRequest(const std::string& request);

private:
    std::unique_ptr<class LlmProvider> provider_;
    static std::string buildPrompt(const std::string& request);
    static std::optional<LlmPlan> parseProviderResponse(const std::string& response);
};
