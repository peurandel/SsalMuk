#include "screen_analyzer.h"
#include <iostream>

ScreenAnalyzer::ScreenAnalyzer() {
}

ScreenAnalyzer::~ScreenAnalyzer() {
}

void ScreenAnalyzer::analyzeScreen() {
    std::cout << "화면 분석: screen.png 파일을 기반으로 UI 상태를 확인합니다.\n";
    std::cout << "(실제 OCR/이미지 인식 엔진을 나중에 통합하세요.)\n";
}
