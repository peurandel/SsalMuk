#pragma once

#include "llm_provider.h"
#include <string>

class OllamaProvider : public LlmProvider {
public:
    OllamaProvider(std::string apiUrl, std::string model);
    std::optional<std::string> requestCompletion(const std::string& prompt) override;

private:
    std::string apiUrl_;
    std::string model_;
};
