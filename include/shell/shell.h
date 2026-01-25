#pragma once
#include <string>
#include <vector>

class Kernel;

class Shell {
public:
    explicit Shell(Kernel& kernel);
    void run();

private:
    Kernel& kernel_;
    bool running_;

    std::vector<std::string> parse_command(const std::string& input);
    void execute_command(const std::vector<std::string>& args);
    void execute_script(const std::string& filename);
};
