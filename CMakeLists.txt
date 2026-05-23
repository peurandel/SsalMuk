cmake_minimum_required(VERSION 3.15)
project(SsalMuk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SsalMuk
    src/main.cpp
    src/llm_agent.cpp
    src/ollama_provider.cpp
    src/scrcpy_controller.cpp
    src/screen_analyzer.cpp
)

add_executable(llm_parse_test
    src/llm_parse_test.cpp
    src/llm_agent.cpp
    src/ollama_provider.cpp
)

add_executable(SsalMukUI
    src/main_vulkan.cpp
    src/vulkan_window.cpp
)

add_executable(SsalMukInstaller
    src/installer.cpp
)

find_package(CURL REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

target_include_directories(SsalMuk PRIVATE src)
target_include_directories(llm_parse_test PRIVATE src)
target_include_directories(SsalMukUI PRIVATE src)
target_include_directories(SsalMukInstaller PRIVATE src)

target_link_libraries(SsalMuk PRIVATE CURL::libcurl)
target_link_libraries(llm_parse_test PRIVATE CURL::libcurl)
target_link_libraries(SsalMukUI PRIVATE Vulkan::Vulkan glfw)

target_compile_features(SsalMukInstaller PRIVATE cxx_std_17)

if(WIN32)
    target_compile_definitions(SsalMuk PRIVATE PLATFORM_WINDOWS)
    target_compile_definitions(SsalMukUI PRIVATE PLATFORM_WINDOWS)
    target_compile_definitions(SsalMukInstaller PRIVATE PLATFORM_WINDOWS)
endif()
