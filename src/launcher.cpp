#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int argc, char* argv[]) {
    std::filesystem::path launcherPath(argv[0]);
    if (launcherPath.empty()) {
        launcherPath = std::filesystem::current_path();
    } else if (!launcherPath.is_absolute()) {
        launcherPath = std::filesystem::current_path() / launcherPath;
    }
    launcherPath = std::filesystem::absolute(launcherPath).parent_path();

    std::filesystem::path target = launcherPath / "SsalMuk";
    if (!std::filesystem::exists(target)) {
        target = launcherPath / "build" / "SsalMuk";
    }

    if (!std::filesystem::exists(target)) {
        std::cerr << "실행 파일을 찾을 수 없습니다. 먼저 빌드된 SsalMuk 바이너리를 생성하세요.\n";
        std::cerr << "예: cmake --build build --target SsalMuk" << std::endl;
        return 1;
    }

    std::vector<std::string> args;
    args.push_back(target.string());
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    std::vector<char*> cargs;
    cargs.reserve(args.size() + 1);
    for (auto& arg : args) {
        cargs.push_back(arg.data());
    }
    cargs.push_back(nullptr);

#ifndef _WIN32
    execv(target.c_str(), cargs.data());
    std::perror("execv");
    return 1;
#else
    std::string command = '"' + target.string() + '"';
    for (int i = 1; i < argc; ++i) {
        command += " ";
        command += argv[i];
    }
    int result = std::system(command.c_str());
    return result;
#endif
}
