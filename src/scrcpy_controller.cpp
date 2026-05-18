#include "scrcpy_controller.h"
#include <cstdlib>
#include <iostream>

ScrcpyController::ScrcpyController() {
}

ScrcpyController::~ScrcpyController() {
}

bool ScrcpyController::launch() {
    std::cout << "scrcpy 실행 중..." << std::endl;
#ifdef _WIN32
    int result = std::system("start /B scrcpy");
#else
    int result = std::system("scrcpy &");
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
    std::string escaped = text;
    for (char& c : escaped) {
        if (c == ' ') c = '+';
    }
    std::string command = "adb shell input text \"" + escaped + "\"";
    return std::system(command.c_str()) == 0;
}

std::optional<std::string> ScrcpyController::captureScreen() {
    std::cout << "화면 캡처를 시도합니다..." << std::endl;
    const char* path = "screen.png";
    std::string command = "adb exec-out screencap -p > ";
    command += path;
    if (std::system(command.c_str()) == 0) {
        return std::string(path);
    }
    return std::nullopt;
}
