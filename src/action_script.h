#pragma once

#include <optional>
#include <string>
#include <vector>

struct LlmPlan;

struct ActionCommand {
    std::string action;
    std::optional<int> x;
    std::optional<int> y;
    std::string text;
    int waitMs = 0;
};

class ScrcpyController;
class ScreenAnalyzer;

class ActionScript {
public:
    ActionScript();
    bool parse(const std::string& source);
    bool loadFromPlan(const LlmPlan& plan);
    bool execute(ScrcpyController& controller, ScreenAnalyzer* analyzer = nullptr, bool dryRun = false);
    const std::vector<ActionCommand>& commands() const;
    std::string lastError() const;

private:
    static std::string trim(const std::string& input);
    static std::string unquote(const std::string& value);

    std::string lastError_;
    std::vector<ActionCommand> commands_;
};
