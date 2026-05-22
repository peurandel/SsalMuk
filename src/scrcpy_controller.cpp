#include "scrcpy_controller.h"
#include <cstdlib>
#include <iostream>

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
    escaped.push_back('\'');
    return escaped;
#endif
}

ScrcpyController::ScrcpyController() {}

ScrcpyController::~ScrcpyController() {}

bool ScrcpyController::launch() {
    std::cout << "scrcpy 실행 중..." << std::endl;
#ifdef _WIN32
    int result = std::system("start /B scrcpy");
#else
    int result = std::system("scrcpy >/dev/null 2>&1 &");
#endif
    return result == 0;
}

bool ScrcpyController::tap(int x, int y) {
    std::cout << "탭 이벤트: (" << x << ", " << y << ")" << std::endl;
    std::string command = "adb shell input tap " + std::to_string(x) + " " + std::to_string(y);
    return std::system(command.c_str()) == 0;
}

bool ScrcpyController::typeText(const std::string& text) {
    std::cout << "텍스트 입력: " << text << std::endl;
    std::string escaped;
    for (char c : text) {
        switch (c) {
            case ' ': escaped += "%s"; break;
            case '%': escaped += "%25"; break;
            case '\n': escaped += "%0A"; break;
            case '\r': escaped += "%0D"; break;
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            default: escaped.push_back(c); break;
        }
    }
    std::string command = "adb shell input text " + shellEscapeArg(escaped);
    return std::system(command.c_str()) == 0;
}

std::optional<std::string> ScrcpyController::captureScreen() {
    std::cout << "화면 캡처를 시도합니다..." << std::endl;
    const std::string path = "screen.png";
    std::string command = "adb exec-out screencap -p > " + shellEscapeArg(path);
    if (std::system(command.c_str()) == 0) {
        return path;
    }
    return std::nullopt;
}
