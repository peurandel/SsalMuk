#include "llm_agent.h"
#include "ollama_provider.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

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
    prompt << "사용자의 명령을 분석하여, 수행 전략과 각 단계별 실행 계획을 반환하세요. ";
    prompt << "먼저 간단한 자연어 설명을 제공하고, 그 다음에 유효한 JSON 블록을 출력하세요. ";
    prompt << "JSON은 다음 스키마를 따라야 합니다:\n";
    prompt << "{\n  \"description\": \"...\",\n  \"steps\": [\n    {\"action\": \"tap\", \"x\": 100, \"y\": 200},\n    {\"action\": \"type\", \"text\": \"Hello\"},\n    {\"action\": \"capture\"}\n  ]\n}\n";
    prompt << "가능하면 tap/type/capture의 구조화된 JSON 계획을 사용하고, 간단한 자연어 설명은 JSON 위나 아래에 포함할 수 있습니다.\n";
    prompt << "사용자 명령: " << request;
    return prompt.str();
}

static std::string trimString(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), value.end());
    return value;
}

struct JsonValue {
    enum class Type { Null, Bool, Number, String, Array, Object } type = Type::Null;
    bool boolValue = false;
    double numberValue = 0;
    std::string stringValue;
    std::vector<JsonValue> arrayValue;
    std::map<std::string, JsonValue> objectValue;
};

class JsonParser {
public:
    JsonParser(const std::string& text) : text_(text), pos_(0) {}

    std::optional<JsonValue> parseValue() {
        skipWhitespace();
        if (pos_ >= text_.size()) {
            return std::nullopt;
        }
        char c = text_[pos_];
        if (c == '{') {
            return parseObject();
        }
        if (c == '[') {
            return parseArray();
        }
        if (c == '"') {
            return parseString();
        }
        if (c == 't' || c == 'f') {
            return parseBool();
        }
        if (c == 'n') {
            return parseNull();
        }
        return parseNumber();
    }

private:
    void skipWhitespace() {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) {
            pos_++;
        }
    }

    std::optional<JsonValue> parseObject() {
        JsonValue result;
        result.type = JsonValue::Type::Object;
        pos_++; // skip '{'
        skipWhitespace();
        if (pos_ < text_.size() && text_[pos_] == '}') {
            pos_++;
            return result;
        }
        while (pos_ < text_.size()) {
            auto keyValue = parseString();
            if (!keyValue || keyValue->type != JsonValue::Type::String) {
                return std::nullopt;
            }
            std::string key = keyValue->stringValue;
            skipWhitespace();
            if (pos_ >= text_.size() || text_[pos_] != ':') {
                return std::nullopt;
            }
            pos_++;
            skipWhitespace();
            auto value = parseValue();
            if (!value) {
                return std::nullopt;
            }
            result.objectValue.emplace(std::move(key), std::move(*value));
            skipWhitespace();
            if (pos_ >= text_.size()) {
                return std::nullopt;
            }
            if (text_[pos_] == '}') {
                pos_++;
                return result;
            }
            if (text_[pos_] != ',') {
                return std::nullopt;
            }
            pos_++;
            skipWhitespace();
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parseArray() {
        JsonValue result;
        result.type = JsonValue::Type::Array;
        pos_++;
        skipWhitespace();
        if (pos_ < text_.size() && text_[pos_] == ']') {
            pos_++;
            return result;
        }
        while (pos_ < text_.size()) {
            auto value = parseValue();
            if (!value) {
                return std::nullopt;
            }
            result.arrayValue.push_back(std::move(*value));
            skipWhitespace();
            if (pos_ >= text_.size()) {
                return std::nullopt;
            }
            if (text_[pos_] == ']') {
                pos_++;
                return result;
            }
            if (text_[pos_] != ',') {
                return std::nullopt;
            }
            pos_++;
            skipWhitespace();
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parseString() {
        if (pos_ >= text_.size() || text_[pos_] != '"') {
            return std::nullopt;
        }
        pos_++;
        JsonValue result;
        result.type = JsonValue::Type::String;
        while (pos_ < text_.size()) {
            char c = text_[pos_++];
            if (c == '"') {
                return result;
            }
            if (c == '\\') {
                if (pos_ >= text_.size()) {
                    return std::nullopt;
                }
                char escapeChar = text_[pos_++];
                switch (escapeChar) {
                    case '"': result.stringValue.push_back('"'); break;
                    case '\\': result.stringValue.push_back('\\'); break;
                    case '/': result.stringValue.push_back('/'); break;
                    case 'b': result.stringValue.push_back('\b'); break;
                    case 'f': result.stringValue.push_back('\f'); break;
                    case 'n': result.stringValue.push_back('\n'); break;
                    case 'r': result.stringValue.push_back('\r'); break;
                    case 't': result.stringValue.push_back('\t'); break;
                    case 'u': {
                        if (pos_ + 4 > text_.size()) {
                            return std::nullopt;
                        }
                        std::string hex = text_.substr(pos_, 4);
                        pos_ += 4;
                        unsigned int codePoint = 0;
                        std::stringstream ss;
                        ss << std::hex << hex;
                        ss >> codePoint;
                        if (codePoint <= 0x7F) {
                            result.stringValue.push_back(static_cast<char>(codePoint));
                        }
                        break;
                    }
                    default:
                        result.stringValue.push_back(escapeChar);
                        break;
                }
            } else {
                result.stringValue.push_back(c);
            }
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parseNumber() {
        size_t start = pos_;
        if (pos_ < text_.size() && (text_[pos_] == '-' || text_[pos_] == '+')) {
            pos_++;
        }
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
            pos_++;
        }
        if (pos_ < text_.size() && text_[pos_] == '.') {
            pos_++;
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                pos_++;
            }
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            pos_++;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) {
                pos_++;
            }
            while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
                pos_++;
            }
        }
        std::string token = text_.substr(start, pos_ - start);
        if (token.empty()) {
            return std::nullopt;
        }
        JsonValue result;
        result.type = JsonValue::Type::Number;
        result.numberValue = std::stod(token);
        return result;
    }

    std::optional<JsonValue> parseBool() {
        if (text_.compare(pos_, 4, "true") == 0) {
            pos_ += 4;
            JsonValue result;
            result.type = JsonValue::Type::Bool;
            result.boolValue = true;
            return result;
        }
        if (text_.compare(pos_, 5, "false") == 0) {
            pos_ += 5;
            JsonValue result;
            result.type = JsonValue::Type::Bool;
            result.boolValue = false;
            return result;
        }
        return std::nullopt;
    }

    std::optional<JsonValue> parseNull() {
        if (text_.compare(pos_, 4, "null") == 0) {
            pos_ += 4;
            JsonValue result;
            result.type = JsonValue::Type::Null;
            return result;
        }
        return std::nullopt;
    }

    const std::string& text_;
    size_t pos_;
};

static std::optional<std::string> extractJsonBlock(const std::string& text) {
    size_t start = text.find('{');
    while (start != std::string::npos) {
        size_t depth = 0;
        bool inString = false;
        bool escaping = false;
        for (size_t i = start; i < text.size(); ++i) {
            char c = text[i];
            if (inString) {
                if (escaping) {
                    escaping = false;
                } else if (c == '\\') {
                    escaping = true;
                } else if (c == '"') {
                    inString = false;
                }
            } else {
                if (c == '"') {
                    inString = true;
                } else if (c == '{') {
                    depth++;
                } else if (c == '}') {
                    if (depth > 0) {
                        depth--;
                        if (depth == 0) {
                            return text.substr(start, i - start + 1);
                        }
                    }
                }
            }
        }
        start = text.find('{', start + 1);
    }
    return std::nullopt;
}

static std::optional<LlmStep> stepFromJson(const JsonValue& stepValue) {
    if (stepValue.type != JsonValue::Type::Object) {
        return std::nullopt;
    }
    auto itAction = stepValue.objectValue.find("action");
    if (itAction == stepValue.objectValue.end() || itAction->second.type != JsonValue::Type::String) {
        return std::nullopt;
    }
    std::string action = itAction->second.stringValue;
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);
    LlmStep step;
    step.action = action;
    if (action == "tap") {
        auto xIt = stepValue.objectValue.find("x");
        auto yIt = stepValue.objectValue.find("y");
        if (xIt == stepValue.objectValue.end() || yIt == stepValue.objectValue.end()) {
            return std::nullopt;
        }
        if (xIt->second.type != JsonValue::Type::Number || yIt->second.type != JsonValue::Type::Number) {
            return std::nullopt;
        }
        step.x = static_cast<int>(std::round(xIt->second.numberValue));
        step.y = static_cast<int>(std::round(yIt->second.numberValue));
    } else if (action == "type") {
        auto textIt = stepValue.objectValue.find("text");
        if (textIt == stepValue.objectValue.end() || textIt->second.type != JsonValue::Type::String) {
            return std::nullopt;
        }
        step.text = textIt->second.stringValue;
    }
    return step;
}

static std::optional<LlmPlan> parseLegacySteps(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    std::vector<LlmStep> steps;
    while (std::getline(stream, line)) {
        line = trimString(line);
        if (line.empty()) {
            continue;
        }
        std::string normalized = line;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
        if (normalized.rfind("tap ", 0) == 0) {
            int x, y;
            if (sscanf(line.c_str(), "tap %d %d", &x, &y) == 2) {
                LlmStep step;
                step.action = "tap";
                step.x = x;
                step.y = y;
                steps.push_back(step);
            }
        } else if (normalized.rfind("type ", 0) == 0) {
            auto pos = line.find(' ');
            if (pos != std::string::npos) {
                LlmStep step;
                step.action = "type";
                step.text = line.substr(pos + 1);
                steps.push_back(step);
            }
        } else if (normalized == "capture") {
            LlmStep step;
            step.action = "capture";
            steps.push_back(step);
        }
    }
    if (steps.empty()) {
        return std::nullopt;
    }
    return LlmPlan{"LLM에서 생성한 화면 조작 계획", steps};
}

static std::string extractJsonStringValue(const std::string& text, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    auto pos = text.find(needle);
    if (pos == std::string::npos) {
        return std::string();
    }
    pos = text.find(':', pos);
    if (pos == std::string::npos) {
        return std::string();
    }
    pos = text.find('"', pos);
    if (pos == std::string::npos) {
        return std::string();
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
    auto content = extractJsonStringValue(text, "content");
    if (!content.empty()) {
        return content;
    }
    auto textValue = extractJsonStringValue(text, "text");
    if (!textValue.empty()) {
        return textValue;
    }
    return text;
}

std::optional<LlmPlan> LlmAgent::parseProviderResponse(const std::string& response) {
    std::string content = extractTextFromJson(response);
    if (auto jsonBlock = extractJsonBlock(content)) {
        JsonParser parser(*jsonBlock);
        if (auto rootValue = parser.parseValue()) {
            if (rootValue->type == JsonValue::Type::Object) {
                std::string description = "LLM에서 생성한 화면 조작 계획";
                auto descriptionIt = rootValue->objectValue.find("description");
                if (descriptionIt != rootValue->objectValue.end() && descriptionIt->second.type == JsonValue::Type::String) {
                    description = descriptionIt->second.stringValue;
                }
                std::vector<LlmStep> steps;
                auto stepsIt = rootValue->objectValue.find("steps");
                if (stepsIt != rootValue->objectValue.end() && stepsIt->second.type == JsonValue::Type::Array) {
                    for (const auto& stepValue : stepsIt->second.arrayValue) {
                        if (auto step = stepFromJson(stepValue)) {
                            steps.push_back(*step);
                        }
                    }
                }
                if (!steps.empty()) {
                    return LlmPlan{description, steps};
                }
            }
        }
    }
    return parseLegacySteps(content);
}
