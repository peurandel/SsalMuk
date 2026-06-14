#include "action_script.h"
#include "llm_agent.h"
#include "screen_analyzer.h"
#include "scrcpy_controller.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <thread>
#include <vector>

ActionScript::ActionScript() {}

std::string ActionScript::trim(const std::string& input) {
    std::string result = input;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), result.end());
    return result;
}

std::string ActionScript::unquote(const std::string& value) {
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        std::string result;
        bool escaping = false;
        for (size_t i = 1; i + 1 < value.size(); i++) {
            char c = value[i];
            if (escaping) {
                switch (c) {
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    case '\\': result.push_back('\\'); break;
                    case '"': result.push_back('"'); break;
                    default: result.push_back(c); break;
                }
                escaping = false;
            } else if (c == '\\') {
                escaping = true;
            } else {
                result.push_back(c);
            }
        }
        return result;
    }
    return value;
}

bool ActionScript::parse(const std::string& source) {
    commands_.clear();
    lastError_.clear();

    std::istringstream stream(source);
    std::string line;
    int lineNumber = 0;

    while (std::getline(stream, line)) {
        lineNumber++;
        line = trim(line);
        if (line.empty() || line.rfind("#", 0) == 0) {
            continue;
        }

        std::istringstream tokens(line);
        std::string opcode;
        if (!(tokens >> opcode)) {
            continue;
        }
        std::transform(opcode.begin(), opcode.end(), opcode.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        ActionCommand command;
        command.action = opcode;

        if (opcode == "tap") {
            int x, y;
            if (!(tokens >> x >> y)) {
                lastError_ = "Line " + std::to_string(lineNumber) + ": tap 명령은 두 개의 정수 좌표가 필요합니다.";
                return false;
            }
            command.x = x;
            command.y = y;
        } else if (opcode == "type") {
            std::string rest;
            std::getline(tokens, rest);
            rest = trim(rest);
            if (rest.empty()) {
                lastError_ = "Line " + std::to_string(lineNumber) + ": type 명령에 텍스트가 필요합니다.";
                return false;
            }
            command.text = unquote(rest);
        } else if (opcode == "wait") {
            int ms;
            if (!(tokens >> ms)) {
                lastError_ = "Line " + std::to_string(lineNumber) + ": wait 명령은 밀리초 정수값이 필요합니다.";
                return false;
            }
            command.waitMs = ms;
        } else if (opcode == "capture") {
            // no additional arguments
        } else {
            lastError_ = "Line " + std::to_string(lineNumber) + ": 알 수 없는 명령 '" + opcode + "'입니다.";
            return false;
        }

        commands_.push_back(std::move(command));
    }

    return true;
}

bool ActionScript::execute(ScrcpyController& controller, ScreenAnalyzer* analyzer, bool dryRun) {
    for (const auto& command : commands_) {
        if (command.action == "tap") {
            if (!command.x || !command.y) {
                lastError_ = "tap 명령에 좌표가 없습니다.";
                return false;
            }
            if (!dryRun && !controller.tap(*command.x, *command.y)) {
                lastError_ = controller.lastError();
                return false;
            }
        } else if (command.action == "type") {
            if (command.text.empty()) {
                lastError_ = "type 명령에 텍스트가 없습니다.";
                return false;
            }
            if (!dryRun && !controller.typeText(command.text)) {
                lastError_ = controller.lastError();
                return false;
            }
        } else if (command.action == "wait") {
            if (!dryRun) {
                std::this_thread::sleep_for(std::chrono::milliseconds(command.waitMs));
            }
        } else if (command.action == "capture") {
            if (!dryRun) {
                auto path = controller.captureScreen();
                if (!path) {
                    lastError_ = controller.lastError();
                    return false;
                }
                if (analyzer) {
                    analyzer->analyzeScreen(*path);
                }
            }
        }
    }
    return true;
}

bool ActionScript::loadFromPlan(const LlmPlan& plan) {
    commands_.clear();
    lastError_.clear();

    for (const auto& step : plan.steps) {
        ActionCommand command;
        command.action = step.action;
        if (command.action == "tap") {
            if (!step.x || !step.y) {
                lastError_ = "Plan에 tap 좌표가 없습니다.";
                return false;
            }
            command.x = step.x;
            command.y = step.y;
        } else if (command.action == "type") {
            if (step.text.empty()) {
                lastError_ = "Plan에 type 텍스트가 없습니다.";
                return false;
            }
            command.text = step.text;
        } else if (command.action == "capture") {
            // no extra fields
        } else if (command.action == "wait") {
            if (step.x) {
                command.waitMs = *step.x;
            } else {
                lastError_ = "Plan에 wait 시간이 없습니다.";
                return false;
            }
        } else {
            lastError_ = "지원하지 않는 액션: " + command.action;
            return false;
        }
        commands_.push_back(std::move(command));
    }
    return true;
}

const std::vector<ActionCommand>& ActionScript::commands() const {
    return commands_;
}

std::string ActionScript::lastError() const {
    return lastError_;
}
