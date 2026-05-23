#include "scrcpy_controller.h"
#include <cstdlib>
#include <iostream>
#include <cstring>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <errno.h>
#include <string.h>
#endif

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
    escaped += "'";
    return escaped;
#endif
}

static bool commandExists(const std::string& command) {
#ifdef _WIN32
    std::string check = "where " + command + " >nul 2>&1";
#else
    std::string check = "command -v " + command + " >/dev/null 2>&1";
#endif
    return std::system(check.c_str()) == 0;
}

void ScrcpyController::setLastError(const std::string& message) {
    lastError_ = message;
}

std::string ScrcpyController::lastError() const {
    return lastError_;
}

ScrcpyController::ScrcpyController() {}

ScrcpyController::~ScrcpyController() {}

#ifndef _WIN32
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
#endif

bool ScrcpyController::launch() {
    setLastError("");
    std::cout << "scrcpy 실행 중..." << std::endl;
    if (!commandExists("scrcpy")) {
        setLastError("scrcpy 명령어를 찾을 수 없습니다. scrcpy가 설치되어 있고 PATH에 포함되어 있는지 확인하세요.");
        return false;
    }
#ifdef _WIN32
    int result = std::system("start /B scrcpy");
    if (result != 0) {
        setLastError("scrcpy 실행 명령을 시작하지 못했습니다.");
        return false;
    }
    return true;
#else
#ifdef __linux__
    pid_t pid = fork();
    if (pid == -1) {
        setLastError(std::string("fork 실패: ") + std::strerror(errno));
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
    if (result != 0) {
        setLastError("scrcpy를 백그라운드로 실행하지 못했습니다.");
        return false;
    }
    return true;
#endif
#endif
}

bool ScrcpyController::tap(int x, int y) {
    setLastError("");
    std::cout << "탭 이벤트: (" << x << ", " << y << ")" << std::endl;
    if (!commandExists("adb")) {
        setLastError("adb 명령어를 찾을 수 없습니다. Android 디바이스 USB 디버깅이 활성화되어 있고 adb가 설치되어 있는지 확인하세요.");
        return false;
    }
#ifndef _WIN32
    std::vector<std::string> args = {"adb", "shell", "input", "tap", std::to_string(x), std::to_string(y)};
    int result = runProcess(args);
    if (result != 0) {
        setLastError("adb tap 명령 실행에 실패했습니다. exit code=" + std::to_string(result));
        return false;
    }
    return true;
#else
    std::string command = "adb shell input tap " + std::to_string(x) + " " + std::to_string(y);
    int result = std::system(command.c_str());
    if (result != 0) {
        setLastError("adb tap 실행에 실패했습니다. exit code=" + std::to_string(result));
        return false;
    }
    return true;
#endif
}

bool ScrcpyController::typeText(const std::string& text) {
    setLastError("");
    std::cout << "텍스트 입력: " << text << std::endl;
    if (!commandExists("adb")) {
        setLastError("adb 명령어를 찾을 수 없습니다. Android 디바이스 USB 디버깅이 활성화되어 있고 adb가 설치되어 있는지 확인하세요.");
        return false;
    }
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
#ifndef _WIN32
    std::vector<std::string> args = {"adb", "shell", "input", "text", escaped};
    int result = runProcess(args);
    if (result != 0) {
        setLastError("adb text 입력 명령 실행에 실패했습니다. exit code=" + std::to_string(result));
        return false;
    }
    return true;
#else
    std::string command = "adb shell input text " + shellEscapeArg(escaped);
    int result = std::system(command.c_str());
    if (result != 0) {
        setLastError("adb text 실행에 실패했습니다. exit code=" + std::to_string(result));
        return false;
    }
    return true;
#endif
}

std::optional<std::string> ScrcpyController::captureScreen() {
    setLastError("");
    std::cout << "화면 캡처를 시도합니다..." << std::endl;
    const std::string path = "screen.png";
    if (!commandExists("adb")) {
        setLastError("adb 명령어를 찾을 수 없습니다. Android 디바이스 USB 디버깅이 활성화되어 있고 adb가 설치되어 있는지 확인하세요.");
        return std::nullopt;
    }
#ifndef _WIN32
    std::vector<std::string> args = {"adb", "exec-out", "screencap", "-p"};
    if (runProcessCaptureToFile(args, path)) {
        return path;
    }
    setLastError("adb 화면 캡처 명령 실행에 실패했습니다.");
    return std::nullopt;
#else
    std::string command = "adb exec-out screencap -p > " + shellEscapeArg(path);
    int result = std::system(command.c_str());
    if (result == 0) {
        return path;
    }
    setLastError("adb 화면 캡처 실행에 실패했습니다. exit code=" + std::to_string(result));
    return std::nullopt;
#endif
}
