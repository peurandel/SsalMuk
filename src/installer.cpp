#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace fs = std::filesystem;

#ifdef _WIN32
static std::string joinPath(const std::string& left, const std::string& right) {
    return left + "\\" + right;
}

static std::string defaultInstallDirectory() {
    const char* programFiles = std::getenv("ProgramFiles");
    if (programFiles && programFiles[0] != '\0') {
        return joinPath(programFiles, "SsalMuk");
    }
    return "C:\\Program Files\\SsalMuk";
}

static std::string defaultBinarySuffix() {
    return ".exe";
}
#else
static std::string joinPath(const std::string& left, const std::string& right) {
    return left + "/" + right;
}

static std::string defaultInstallDirectory() {
    if (geteuid() == 0) {
        return "/opt/SsalMuk";
    }
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return joinPath(std::string(home), ".local/share/SsalMuk");
    }
    return "/tmp/SsalMuk";
}

static std::string defaultBinarySuffix() {
    return "";
}
#endif

static std::string getExecutableName(const std::string& baseName) {
    return baseName + defaultBinarySuffix();
}

static void printUsage() {
    std::cout << "SsalMuk Installer\n";
    std::cout << "사용법: installer [--source <build_dir>] [--dest <install_dir>] [--help]\n";
    std::cout << "  --source  빌드 결과물이 들어있는 디렉터리 (기본: build)\n";
    std::cout << "  --dest    설치 대상 디렉터리 (기본: " << defaultInstallDirectory() << ")\n";
    std::cout << "Installer는 빌드된 SsalMuk 실행 파일과 Vulkan UI 실행 파일을 설치합니다.\n";
}

static bool copyBinary(const fs::path& sourceDir, const std::string& name, const fs::path& destDir) {
    fs::path sourceFile = sourceDir / getExecutableName(name);
    if (!fs::exists(sourceFile)) {
        std::cerr << "빌드된 실행 파일을 찾을 수 없습니다: " << sourceFile << "\n";
        return false;
    }

    fs::path destFile = destDir / getExecutableName(name);
    try {
        fs::copy_file(sourceFile, destFile, fs::copy_options::overwrite_existing);
        std::cout << "설치됨: " << destFile << "\n";
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "파일 복사 실패: " << ex.what() << "\n";
        return false;
    }
}

#ifndef _WIN32
static void createPathSymlink(const fs::path& target, const std::string& linkName) {
    fs::path linkPath = fs::path("/usr/local/bin") / linkName;
    try {
        if (fs::exists(linkPath) || fs::is_symlink(linkPath)) {
            fs::remove(linkPath);
        }
        fs::create_symlink(target, linkPath);
        std::cout << "경로에 심볼릭 링크를 생성했습니다: " << linkPath << " -> " << target << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "심볼릭 링크 생성 실패: " << ex.what() << "\n";
        std::cerr << "/usr/local/bin에 쓰기 권한이 없으면 관리자 권한으로 설치하세요.\n";
    }
}
#endif

int main(int argc, char* argv[]) {
    fs::path sourceDir = "build";
    fs::path destDir = defaultInstallDirectory();

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
        if (arg == "--source" && i + 1 < argc) {
            sourceDir = argv[++i];
            continue;
        }
        if (arg == "--dest" && i + 1 < argc) {
            destDir = argv[++i];
            continue;
        }
        std::cerr << "알 수 없는 옵션: " << arg << "\n";
        printUsage();
        return 1;
    }

    if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir)) {
        std::cerr << "유효한 빌드 디렉터리가 아닙니다: " << sourceDir << "\n";
        return 1;
    }

    try {
        fs::create_directories(destDir);
    } catch (const std::exception& ex) {
        std::cerr << "설치 디렉터리 생성 실패: " << ex.what() << "\n";
        return 1;
    }

    std::vector<std::string> binaries = {"SsalMuk", "SsalMukUI"};
    bool success = true;
    for (const auto& binary : binaries) {
        success &= copyBinary(sourceDir, binary, destDir);
    }

#ifndef _WIN32
    if (success) {
        if (geteuid() == 0) {
            createPathSymlink(destDir / getExecutableName("SsalMuk"), "ssalmuk");
            createPathSymlink(destDir / getExecutableName("SsalMukUI"), "ssalmuk-ui");
        } else {
            std::cout << "루트 권한이 아닙니다. /usr/local/bin 심볼릭 링크는 생성되지 않았습니다.\n";
        }
    }
#endif

    if (!success) {
        std::cerr << "설치 중 오류가 발생했습니다.\n";
        return 1;
    }

    std::cout << "SsalMuk가 설치되었습니다.\n";
    std::cout << "설치 위치: " << destDir << "\n";
#ifndef _WIN32
    std::cout << "/usr/local/bin/ssalmuk 또는 /usr/local/bin/ssalmuk-ui로 실행할 수 있습니다.\n";
#else
    std::cout << "설치된 폴더를 PATH에 추가하거나 바로가기 링크를 생성하세요.\n";
#endif

    return 0;
}
