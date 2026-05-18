#include "llm_agent.h"
#include "ollama_provider.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>

LlmAgent::LlmAgent(const std::string& apiUrl, const std::string& model)
    : provider_(std::make_unique<OllamaProvider>(apiUrl, model)) {
}

LlmAgent::~LlmAgent() {
}

std::optional<LlmPlan> LlmAgent::analyzeUserRequest(const std::string& request) {
    const std::string prompt = buildPrompt(request);
    auto response = provider_->requestCompletion(prompt);
    if (!response) {
        return std::nullopt;
    }
    return parseProviderResponse(*response);
}

std::string LlmAgent::buildPrompt(const std::string& request) {
    std::ostringstream prompt;
    prompt << "당신은 Android 화면 자동화를 위한 보조 도우미입니다. ";
    prompt << "사용자는 scrcpy로 연결된 Android 기기를 조작하려 합니다. ";
    prompt << "사용자의 명령을 분석하여, 각 단계별로 수행할 수 있는 화면 조작 계획을 반환하세요. ";
    prompt << "사용 가능한 단계는 다음과 같습니다: \"tap x y\", \"type TEXT\", \"capture\". ";
    prompt << "각 단계는 별도의 줄에 작성하고, 불필요한 설명은 생략하세요.\n";
    prompt << "사용자 명령: " << request;
    return prompt.str();
}

static std::string trimString(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), value.end());
    return value;
}

static std::optional<std::string> extractJsonStringValue(const std::string& text, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    auto pos = text.find(needle);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = text.find(':', pos);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = text.find('"', pos);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    std::string extracted;
    bool escaping = false;
    for (size_t i = pos + 1; i < text.size(); ++i) {
        char c = text[i];
        if (escaping) {
            switch (c) {
            case '"': extracted.push_back('"'); break;
            case '\\': extracted.push_back('\\'); break;
            case 'n': extracted.push_back('\n'); break;
            case 'r': extracted.push_back('\r'); break;
            case 't': extracted.push_back('\t'); break;
            default: extracted.push_back(c); break;
            }
            escaping = false;
        } else if (c == '\\') {
            escaping = true;
        } else if (c == '"') {
            break;
        } else {
            extracted.push_back(c);
        }
    }
    return extracted;
}

static std::string extractTextFromJson(const std::string& text) {
    if (auto content = extractJsonStringValue(text, "content"); content && !content->empty()) {
        return *content;
    }
    if (auto textValue = extractJsonStringValue(text, "text"); textValue && !textValue->empty()) {
        return *textValue;
    }
    return text;
}

std::optional<LlmPlan> LlmAgent::parseProviderResponse(const std::string& response) {
    std::string content = extractTextFromJson(response);
    std::istringstream stream(content);
    std::string line;
    std::vector<std::string> steps;

    while (std::getline(stream, line)) {
        line = trimString(line);
        if (line.empty()) {
            continue;
        }

        std::string normalized = line;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);

        if (normalized.rfind("tap ", 0) == 0 || normalized.rfind("type ", 0) == 0 || normalized == "capture") {
            steps.push_back(line);
        }
    }

    if (steps.empty()) {
        return std::nullopt;
    }

    return LlmPlan{"LLM에서 생성한 화면 조작 계획", steps};
}
