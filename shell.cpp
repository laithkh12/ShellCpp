#include "Shell.h"

Shell::Shell() {
    variables = load_variables("laith.txt");
}

std::vector<std::string> Shell::tokenize(const std::string &input) {
    std::vector<std::string> commandArgs;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        commandArgs.push_back(token);
    }
    return commandArgs;
}

#include <fcntl.h> // for open() function

#include <unistd.h>
#include <fcntl.h>

std::string trim(const std::string &str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        start++;
    }

    auto end = str.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

void Shell::execute_command(const std::string &command, bool background) {
    std::vector<std::string> commands;
    std::istringstream iss(command);
    std::string token;
    while (std::getline(iss, token, '|')) {
        commands.push_back(token);
    }

    int num_commands = commands.size();
    int pipes[num_commands - 1][2]; // One fewer pipe than commands

    for (int i = 0; i < num_commands - 1; ++i) {
        if (pipe(pipes[i]) == -1) {
            std::cerr << "Pipe creation failed." << std::endl;
            return;
        }
    }

    for (int i = 0; i < num_commands; ++i) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            if (i != 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO); // Set stdin from previous pipe
            }
            if (i != num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO); // Set stdout to pipe
            }

            // Close all pipe ends in child process
            for (int j = 0; j < num_commands - 1; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Handle input redirection for first command
            if (i == 0) {
                std::size_t input_pos = commands[i].find('<');
                if (input_pos != std::string::npos) {
                    std::string input_file = trim(commands[i].substr(input_pos + 1));
                    int fd_input = open(input_file.c_str(), O_RDONLY);
                    if (fd_input == -1) {
                        std::cerr << "Failed to open input file: " << input_file << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd_input, STDIN_FILENO);
                    close(fd_input);
                    commands[i] = commands[i].substr(0, input_pos);
                }
            }

            // Handle output redirection for last command
            if (i == num_commands - 1) {
                std::size_t output_pos = commands[i].find('>');
                if (output_pos != std::string::npos) {
                    std::string output_file = trim(commands[i].substr(output_pos + 1));
                    int fd_output = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (fd_output == -1) {
                        std::cerr << "Failed to open output file: " << output_file << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd_output, STDOUT_FILENO);
                    close(fd_output);
                    commands[i] = commands[i].substr(0, output_pos);
                }
            }

            // Tokenize command
            std::vector<std::string> args = tokenize(commands[i]);

            // Convert arguments to char* array for execvp
            std::vector<char*> argv;
            for (const auto& arg : args) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(nullptr);

            // Execute command
            execvp(argv[0], argv.data());

            // Execution failed if reached here
            std::cerr << "Execution failed." << std::endl;
            exit(EXIT_FAILURE);
        } else if (child_pid < 0) {
            std::cerr << "Failed to fork." << std::endl;
            return;
        }
    }

    // Close all pipe ends in parent process
    for (int i = 0; i < num_commands - 1; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; ++i) {
        int status;
        wait(&status);
    }
}

void Shell::update_job_status() {
    for (auto &job : jobsInBg) {
        if (job.completed) {
            continue;
        }
        int status;
        pid_t result = waitpid(job.pid, &status, WNOHANG);
        switch (result) {
            case -1:
                std::cerr << "Error while waiting for process." << std::endl;
                break;
            case 0:
                break;
            default:
                job.completed = true;
                std::cout << "Background job completed. PID: " << job.pid << std::endl;
                break;
        }
    }
}

void Shell::display_jobs() const {
    std::cout << "Background Jobs:" << std::endl;
    bool hasJobs = false;
    for (const auto &job : jobsInBg) {
        if (!job.completed) {
            hasJobs = true;
            std::cout << "PID: " << job.pid << ", Command: " << job.command << ", Status: " << (job.completed ? "Completed" : "Running") << std::endl;
        }
    }
    if (!hasJobs) {
        std::cout << "No background jobs running." << std::endl;
    }
}

void Shell::change_directory(const std::string &directory) {
    const char* dir = (directory == "~") ? getenv("HOME") : directory.c_str();
    if (dir == nullptr || chdir(dir) != 0) {
        std::cerr << "Failed to change directory to " << directory << std::endl;
    }
}

std::string Shell::resolve_variables(const std::string &input) {
    std::string result = input;
    std::regex re(R"(\$\{?([a-zA-Z0-9_]+)\}?)");
    std::smatch match;
    while (std::regex_search(result, match, re)) {
        std::string variableName = match[1].str();
        const char* variableValue = getenv(variableName.c_str());
        result.replace(match.position(0), match.length(0), variableValue ? variableValue : "");
    }
    return result;
}

std::unordered_map<std::string, std::string> Shell::load_variables(const std::string &filename) {
    std::unordered_map<std::string, std::string> vars;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                vars[key] = value;
            }
        }
        file.close();
    }
    return vars;
}

void Shell::save_command_history(const std::string &command, const std::string &filename) {
    std::ofstream file(filename, std::ios_base::app);
    if (file.is_open()) {
        file << command << std::endl;
        file.close();
    } else {
        std::cerr << "Failed to open history file." << std::endl;
    }
}

void Shell::display_command_history(const std::string &filename) const {
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        int count = 1;
        while (std::getline(file, line)) {
            std::cout << count << ". " << line << std::endl;
            count++;
        }
        file.close();
    } else {
        std::cerr << "Failed to open history file." << std::endl;
    }
}

void Shell::display_prompt() const {
    char currentDir[1024];
    if (getcwd(currentDir, sizeof(currentDir)) != nullptr) {
        std::cout << currentDir << " $ ";
    } else {
        std::cerr << "Cannot get current directory." << std::endl;
        std::cout << "$ ";
    }
}

void Shell::process_input(std::string &userInput) {
    save_command_history(userInput, "history.txt");

    bool backgroundJobs = false;
    if (!userInput.empty() && userInput.back() == '&') {
        backgroundJobs = true;
        userInput.pop_back();
    }

    userInput = resolve_variables(userInput);
    std::vector<std::string> commandArgs = tokenize(userInput);
    update_job_status();

    if (!commandArgs.empty()) {
        const std::string &command = commandArgs[0];
        if (command == "cd") {
            if (commandArgs.size() == 2) {
                change_directory(commandArgs[1]);
            } else {
                std::cerr << "Usage: cd <directory>" << std::endl;
            }
        } else if (command == "myjobs") {
            display_jobs();
        } else if (command == "myhistory") {
            display_command_history("history.txt");
        } else if (command == "exit") {
            exit(0);
        } else {
            std::string commandLine;
            for (const auto &arg : commandArgs) {
                commandLine += arg + " ";
            }
            execute_command(commandLine, backgroundJobs);
        }
    }
}

void Shell::run() {
    std::string input;
    while (true) {
        display_prompt();
        std::getline(std::cin, input);
        if (!input.empty()) {
            process_input(input);
        }
    }
}



