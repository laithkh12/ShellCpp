#ifndef SHELL2_SHELL_H
#define SHELL2_SHELL_H

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <regex>
#include <algorithm>

struct BackgroundJob {
    pid_t pid;
    std::string command;
    bool completed;
};

class Shell {
public:
    Shell();

    void run();

private:
    std::vector<BackgroundJob> jobsInBg;
    std::unordered_map<std::string, std::string> variables;

    std::vector<std::string> tokenize(const std::string &input);

    void execute_command(const std::string &command, bool background);

    void update_job_status();

    void display_jobs() const;

    void change_directory(const std::string &directory);

    std::string resolve_variables(const std::string &input);

    std::unordered_map<std::string, std::string> load_variables(const std::string &filename);

    void save_command_history(const std::string &command, const std::string &filename);

    void display_command_history(const std::string &filename) const;

    void display_prompt() const;

    void process_input(std::string &userInput);


};


#endif //SHELL2_SHELL_H