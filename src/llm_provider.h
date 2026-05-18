#pragma once

#include <optional>
#include <string>

class LlmProvider {
public:
    virtual ~LlmProvider() = default;
    virtual std::optional<std::string> requestCompletion(const std::string& prompt) = 0;
};
