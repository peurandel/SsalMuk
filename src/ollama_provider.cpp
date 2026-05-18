#include "ollama_provider.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

static std::string escapeJsonString(const std::string& input) {
    std::ostringstream escaped;
    for (char c : input) {
        switch (c) {
            case '"': escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b"; break;
            case '\f': escaped << "\\f"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    escaped << "\\u" << std::hex << std::uppercase << (int)c;
                } else {
                    escaped << c;
                }
        }
    }
    return escaped.str();
}

static std::string getEnvValue(const char* name) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : std::string();
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

OllamaProvider::OllamaProvider(std::string apiUrl, std::string model)
    : apiUrl_(std::move(apiUrl)), model_(std::move(model)) {
}

std::optional<std::string> OllamaProvider::requestCompletion(const std::string& prompt) {
    const std::string bodyPath = "ssalmuk_ollama_request.json";
    const std::string responsePath = "ssalmuk_ollama_response.json";

    std::ofstream bodyFile(bodyPath);
    if (!bodyFile.is_open()) {
        std::cerr << "LLM 요청 본문 파일을 생성할 수 없습니다: " << bodyPath << std::endl;
        return std::nullopt;
    }

    std::string escapedPrompt = escapeJsonString(prompt);
    bodyFile << "{\"model\":\"" << model_ << "\","
             << "\"messages\":[{\"role\":\"system\",\"content\":\"You are an Android automation assistant. Respond with a sequence of actions using tap, type, and capture steps.\"},"
             << "{\"role\":\"user\",\"content\":\"" << escapedPrompt << "\"}],"
             << "\"temperature\":0.2,\"max_tokens\":512}";
    bodyFile.close();

    std::string apiKey = getEnvValue("OLLAMA_API_KEY");
    std::string authHeader;
    if (!apiKey.empty()) {
        authHeader = " -H \"Authorization: Bearer " + apiKey + "\"";
    }

    std::string command = "curl -s -X POST -H \"Content-Type: application/json\"" + authHeader + " -d @" + bodyPath + " \"" + apiUrl_ + "\" > " + responsePath;
    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Ollama API 호출에 실패했습니다. 명령: " << command << std::endl;
        return std::nullopt;
    }

    std::ifstream responseFile(responsePath);
    if (!responseFile.is_open()) {
        std::cerr << "LLM 응답 파일을 읽을 수 없습니다: " << responsePath << std::endl;
        return std::nullopt;
    }

    std::ostringstream responseBuffer;
    responseBuffer << responseFile.rdbuf();
    std::string responseText = responseBuffer.str();
    responseFile.close();

    if (responseText.empty()) {
        std::cerr << "Ollama 응답이 비어 있습니다." << std::endl;
        return std::nullopt;
    }

    if (auto content = extractJsonStringValue(responseText, "content"); content && !content->empty()) {
        return content;
    }
    if (auto text = extractJsonStringValue(responseText, "text"); text && !text->empty()) {
        return text;
    }
    return responseText;
}
