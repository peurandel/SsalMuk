#include "scrcpy_controller.h"
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <errno.h>
#include <string.h>

static std::string shellEscapeArg(const std::string& value) {
#ifdef _WIN32
    std::string escaped = "\"";
    for (char c : value) {
        if (c == '"' || c == '%') {
            escaped.push_back('\\');
        }
        escaped.push_back(c);
    }
    escaped.push_back('"');
    return escaped;
#else
    if (value.empty()) {
        return "''";
    }
    std::string escaped = "'";
    for (char c : value) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped.push_back(c);
        }
    }
    escaped.push_back('\'');
    return escaped;
#endif
}

ScrcpyController::ScrcpyController() {}

ScrcpyController::~ScrcpyController() {}

static int runProcess(const std::vector<std::string>& args) {
    if (args.empty()) return -1;
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (const auto &s : args) argv.push_back(const_cast<char*>(s.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid == -1) return -1;
    if (pid == 0) {
        execvp(argv[0], argv.data());
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) return -1;
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
}

static bool runProcessCaptureToFile(const std::vector<std::string>& args, const std::string& outPath) {
    if (args.empty()) return false;
    int pipefd[2];
    if (pipe(pipefd) == -1) return false;

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }
    if (pid == 0) {
        // child: write to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (const auto &s : args) argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }

    // parent: read from pipe and write to file
    close(pipefd[1]);
    int outFd = open(outPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (outFd == -1) {
        close(pipefd[0]);
        // wait for child to avoid zombie
        waitpid(pid, nullptr, 0);
        return false;
    }

    char buffer[4096];
    ssize_t n;
    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        ssize_t w = write(outFd, buffer, n);
        (void)w;
    }
    close(pipefd[0]);
    close(outFd);

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) return false;
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

bool ScrcpyController::launch() {
    std::cout << "scrcpy 실행 중..." << std::endl;
#ifdef _WIN32
    int result = std::system("start /B scrcpy");
#else
#    ifdef __linux__
    pid_t pid = fork();
    if (pid == -1) {
        return false;
    }
    if (pid == 0) {
        setsid();
        execlp("scrcpy", "scrcpy", (char*)NULL);
        _exit(127);
    }
    return true;
#else
    int result = std::system("scrcpy >/dev/null 2>&1 &");
    return result == 0;
#endif
}

bool ScrcpyController::tap(int x, int y) {
    std::cout << "탭 이벤트: (" << x << ", " << y << ")" << std::endl;
    std::vector<std::string> args = {"adb", "shell", "input", "tap", std::to_string(x), std::to_string(y)};
    return runProcess(args) == 0;
}

bool ScrcpyController::typeText(const std::string& text) {
    std::cout << "텍스트 입력: " << text << std::endl;
    std::string escaped;
    for (char c : text) {
        switch (c) {
            case ' ': escaped += "%s"; break;
            case '%': escaped += "%25"; break;
            case '\n': escaped += "%0A"; break;
            case '\r': escaped += "%0D"; break;
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            default: escaped.push_back(c); break;
        }
    }
    std::vector<std::string> args = {"adb", "shell", "input", "text", escaped};
    return runProcess(args) == 0;
}

std::optional<std::string> ScrcpyController::captureScreen() {
    std::cout << "화면 캡처를 시도합니다..." << std::endl;
    const std::string path = "screen.png";
    std::vector<std::string> args = {"adb", "exec-out", "screencap", "-p"};
    if (runProcessCaptureToFile(args, path)) {
        return path;
    }
    return std::nullopt;
}
