#include "utils.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdio.h>
#include <windows.h>

#ifdef _WIN32
char file_sep = ';';
#include <direct.h>
#else
char file_sep = ':';
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

Token::Token(std::string arg, int quote_type) {
    // arg stores the current argument
    this->value = arg;
    // has_space is used to check if the current argument is followed by a space
    this->has_space = false;
    // quote_type is used to check if the current argument is quoted or not
    this->quote_type = quote_type;
}

void Token::set_has_space() { this->has_space = true; }

Tokenizer::Tokenizer(std::string input) {
    this->input = input;
    this->count = 0;
    this->generic_tokens = tokenize();
}

std::string Token::get_without_quotes() {
    if (this->quote_type == 0)
        return this->value;
    return this->value.substr(1, this->value.size() - 2);
}

void Tokenizer::add_token(std::string &curr_arg, std::vector<Token> &tokens,
                          int quote_type) {
    if (curr_arg.empty())
        return;
    Token token = Token(curr_arg, quote_type);
    tokens.push_back(token);
    curr_arg.clear();
}

std::vector<Token> Tokenizer::tokenize(bool escape_backslashes) {
    int n = this->input.size();
    std::string curr_arg = "";
    bool in_double_q = false, in_single_q = false;
    std::vector<Token> tokens;

    for (int i = 0; i < n; i++) {
        if (!in_single_q && input[i] == '\\' && i == n - 1 &&
            escape_backslashes)
            throw new std::exception(
                "Error: Expected escape character after \\");

        if (in_single_q) {
            curr_arg += input[i];
            if (input[i] == '\'') {
                add_token(curr_arg, tokens, 1);
                in_single_q = false;
            }
        } else if (in_double_q) {
            if (input[i] == '\\' && escape_backslashes) {
                curr_arg += input[++i];
                continue;
            }
            curr_arg += input[i];
            if (input[i] == '"') {
                add_token(curr_arg, tokens, 2);
                in_double_q = false;
            }
        } else {
            if (input[i] == ' ') {
                add_token(curr_arg, tokens);
                tokens.back().set_has_space();
            } else if (input[i] == '\\') {
                curr_arg += input[++i];
            } else {
                if (input[i] == '\'') {
                    in_single_q = true;
                    add_token(curr_arg, tokens);
                } else if (input[i] == '"') {
                    in_double_q = true;
                    add_token(curr_arg, tokens);
                }
                curr_arg += input[i];
            }
        }
    }
    if (curr_arg != "")
        add_token(curr_arg, tokens);

    this->command = tokens[0];
    tokens.erase(tokens.begin());
    return tokens;
}

std::vector<Token> Tokenizer::get_tokens() { return generic_tokens; }

std::string Tokenizer::concat_args(bool add_space = false) {
    std::string res = "";
    for (auto tk : this->generic_tokens) {
        res += tk.value;
        if (add_space)
            res += (tk.has_space ? " " : "");
    }
    return res;
}

std::vector<Token> Tokenizer::get_cat_args() { return tokenize(false); }

std::vector<std::string> split_args(std::string str, char delimiter) {
    std::stringstream ss(str);
    std::string item = "";
    std::vector<std::string> tokens;

    while (getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

bool is_shell_builtin(std::string str) {
    std::string builtin_commands[] = {"pwd", "cd", "type", "exit", "echo"};
    for (std::string command : builtin_commands) {
        if (command == str)
            return true;
    }
    return false;
}

std::string get_file_path(char *directory_paths, std::string filename) {
    try {

        std::vector<std::string> extensions = {".exe", ".sh", ".bat", ".cmd"};

        std::vector<std::string> paths =
            split_args(std::string(directory_paths), file_sep);

        for (auto &path : paths) {

            if (!fs::exists(path))
                continue;

            fs::path prog_path = fs::path(path) / filename;

            for (auto &ext : extensions) {
                if (fs::exists(prog_path.string() + ext)) {
                    return prog_path.lexically_normal().string() + ext;
                }
            }
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception in get_file_path: " << ex.what() << std::endl;
    }
    return "";
}

std::string join(std::vector<Token> args, std::string sep) {
    std::string res = "";
    int n = args.size();
    if (n == 0)
        return res;
    for (int i = 0; i < n - 1; i++) {
        res += args[i].value + sep;
    }
    res += args[n - 1].value;
    return res;
}

void print_cmd_type(std::string command, char *directory_paths) {
    try {
        bool is_builtin = is_shell_builtin(command);

        if (is_builtin) {
            std::cout << command << " is a shell builtin" << std::endl;
            return;
        }

        std::string file_path = get_file_path(directory_paths, command);

        if (file_path == "")
            std::cout << command << ": not found" << std::endl;
        else
            std::cout << command << " is " << file_path << std::endl;

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * Custom cat command that handles quoted and unquoted file paths
 * Created this because cat command only works on linux/unix based systems
 */
void custom_cat_cmd(std::vector<Token> args) {

    for (auto file_path : args) {

        try {

            if (file_path.value == " " || file_path.value == "")
                continue;

            std::string path = file_path.get_without_quotes();
            std::ifstream file;
            file.open(path);

            if (!file.is_open()) {
                std::cerr << "Error opening file : " << path << std::endl;
                continue;
            }

            std::cout << file.rdbuf();
            file.close();
        } catch (const std::exception &e) {
            std::cerr << "Exception in custom_cat_cmd: " << e.what()
                      << std::endl;
        }
    }
}

std::string process_exec_input(std::string cmd, std::vector<Token> arguments) {
    std::string p_input = cmd;
    if (cmd.find(' ') != std::string::npos) {
        if (cmd.find('"') != std::string::npos)
            p_input = "'" + cmd + "'";
        else
            p_input = "\"" + cmd + "\"";
    }

    if (arguments.size() > 0)
        p_input += " " + join(arguments, " ");

    return p_input;
}

char *get_home_directory() {
#ifdef _WIN32
    return getenv("USERPROFILE");
#else
    return getenv("HOME");
#endif
}

int execute_cmd_linux(const char *cmd) {
#ifdef _WIN32
    return 0;
#else
    pid_t pid = fork();
    if (pid < 0) {
        std::cout << "Fork failed" << std::endl;
    } else if (pid == 0) {
        int output = execv(cmd, NULL);
        if (output == -1) {
            std::cout << "Error executing command : " << cmd << std::endl;
        }
        return 1;
    } else {
        waitpid(pid, NULL, 0);
        return 0;
    }
    return 0;
#endif
}

int execute_cmd(const char *cmd) {

#ifdef _WIN32
    return execute_cmd_windows(cmd);
#else
    return execute_cmd_linux(cmd);
#endif
}

int execute_cmd_windows(const char *cmd) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process.
    if (!CreateProcess(NULL,          // No module name (use command line)
                       (LPTSTR)(cmd), // Command line
                       NULL,          // Process handle not inheritable
                       NULL,          // Thread handle not inheritable
                       FALSE,         // Set handle inheritance to FALSE
                       0,             // No creation flags
                       NULL,          // Use parent's environment block
                       NULL,          // Use parent's starting directory
                       &si,           // Pointer to STARTUPINFO structure
                       &pi // Pointer to PROCESS_INFORMATION structure
                       )) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        return 0;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 1;
}
