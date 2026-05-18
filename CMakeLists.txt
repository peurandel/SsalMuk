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

target_include_directories(SsalMuk PRIVATE src)

if(WIN32)
    target_compile_definitions(SsalMuk PRIVATE PLATFORM_WINDOWS)
endif()
