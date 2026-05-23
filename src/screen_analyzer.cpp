#include "screen_analyzer.h"
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

ScreenAnalyzer::ScreenAnalyzer() {
}

ScreenAnalyzer::~ScreenAnalyzer() {
}

static std::string shellEscapeArg(const std::string& value) {
#ifdef _WIN32
    std::string escaped = "\"";
    for (char c : value) {
        if (c == '"' || c == '%') {
            escaped.push_back('\\');
        }
        escaped.push_back(c);
    }
    escaped.push_back('"');
    return escaped;
#else
    if (value.empty()) {
        return "''";
    }
    std::string escaped = "'";
    for (char c : value) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped.push_back(c);
        }
    }
    escaped += "'";
    return escaped;
#endif
}

static std::optional<std::string> captureCommandOutput(const std::string& command) {
    std::array<char, 4096> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        return std::nullopt;
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void ScreenAnalyzer::analyzeScreen(const std::string& screenPath) {
    std::cout << "화면 분석: " << screenPath << " 파일을 기반으로 UI 상태를 확인합니다.\n";
    std::cout << "(OCR이 가능한 경우 tesseract를 사용해 텍스트를 추출합니다.)\n";

    std::string command = "tesseract " + shellEscapeArg(screenPath) + " stdout";
#ifdef _WIN32
    command += " 2>nul";
#else
    command += " 2>/dev/null";
#endif

    auto ocrResult = captureCommandOutput(command);
    if (ocrResult && !ocrResult->empty()) {
        std::cout << "OCR 분석 결과:\n" << *ocrResult << std::endl;
        return;
    }

    std::cout << "OCR 결과를 얻지 못했습니다.\n";
    std::cout << "기본 화면 분석은 현재 텍스트 출력과 확장 가능한 구조만 제공합니다.\n";
}
