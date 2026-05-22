#pragma once

#include <string>

class ScreenAnalyzer {
public:
    ScreenAnalyzer();
    ~ScreenAnalyzer();

    void analyzeScreen(const std::string& screenPath);
};
