#include "vulkan_window.h"
#include <iostream>

int main() {
    std::cout << "SsalMuk Vulkan UI 클라이언트를 시작합니다. ESC 또는 창 닫기로 종료합니다.\n";
    VulkanWindow app(1280, 720, "SsalMuk Vulkan UI");
    if (!app.initialize()) {
        std::cerr << "Vulkan UI 초기화에 실패했습니다.\n";
        return 1;
    }

    app.run();
    return 0;
}
