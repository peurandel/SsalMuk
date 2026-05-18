#pragma once

#include <optional>
#include <string>

class ScrcpyController {
public:
    ScrcpyController();
    ~ScrcpyController();

    bool launch();
    bool tap(int x, int y);
    bool typeText(const std::string& text);
    std::optional<std::string> captureScreen();
};
