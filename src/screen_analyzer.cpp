#include "screen_analyzer.h"
#include <iostream>

ScreenAnalyzer::ScreenAnalyzer() {
}

ScreenAnalyzer::~ScreenAnalyzer() {
}

void ScreenAnalyzer::analyzeScreen(const std::string& screenPath) {
    std::cout << "화면 분석: " << screenPath << " 파일을 기반으로 UI 상태를 확인합니다.\n";
    std::cout << "(실제 OCR/이미지 인식 또는 이미지 분류 통합을 위해 확장할 수 있습니다.)\n";
}
