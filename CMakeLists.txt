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

find_package(CURL REQUIRED)

target_include_directories(SsalMuk PRIVATE src)
target_include_directories(llm_parse_test PRIVATE src)
target_link_libraries(SsalMuk PRIVATE CURL::libcurl)
target_link_libraries(llm_parse_test PRIVATE CURL::libcurl)

if(WIN32)
    target_compile_definitions(SsalMuk PRIVATE PLATFORM_WINDOWS)
endif()
