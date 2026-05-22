#include "ollama_provider.h"
#include <curl/curl.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

static std::string getEnvValue(const char* name) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : std::string();
}

static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

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

OllamaProvider::OllamaProvider(std::string apiUrl, std::string model)
    : apiUrl_(std::move(apiUrl)), model_(std::move(model)) {
}

std::optional<std::string> OllamaProvider::requestCompletion(const std::string& prompt) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "libcurl 초기화에 실패했습니다." << std::endl;
        return std::nullopt;
    }

    std::string responseText;
    std::string requestBody;
    requestBody.reserve(prompt.size() + 256);
    requestBody += "{\"model\":\"";
    requestBody += model_;
    requestBody += "\",";
    requestBody += "\"messages\":[{\"role\":\"system\",\"content\":\"You are an Android automation assistant. Respond with a short natural language reasoning, then output a valid JSON object with keys description and steps. Each step must be an object with action and optional x, y, or text fields.\"},{\"role\":\"user\",\"content\":\"";
    requestBody += escapeJsonString(prompt);
    requestBody += "\"}],\"temperature\":0.2,\"max_tokens\":512}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string apiKey = getEnvValue("OLLAMA_API_KEY");
    if (!apiKey.empty()) {
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, apiUrl_.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(requestBody.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseText);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SsalMuk/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Ollama API 호출에 실패했습니다: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }
    if (responseCode < 200 || responseCode >= 300) {
        std::cerr << "Ollama API 응답 코드가 비정상적입니다: " << responseCode << std::endl;
        return std::nullopt;
    }

    if (responseText.empty()) {
        std::cerr << "Ollama 응답이 비어 있습니다." << std::endl;
        return std::nullopt;
    }

    return responseText;
}
