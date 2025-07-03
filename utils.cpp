#include "utils.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdio.h>

#ifdef _WIN32
char file_sep = ';';
#include <direct.h>
#else
char file_sep = ':';
#include <unistd.h>
#endif

namespace fs = std::filesystem;

Token::Token(std::string arg, bool has_space = false) {
    this->arg = arg;
    this->has_space = has_space;
    // arg stores the current argument
    // has_space is used to check if the current argument is followed by a space
    // or not so as to add a space after it in case of echo
}

void Token::set_has_space() { this->has_space = true; }

Tokenizer::Tokenizer(std::string input) {
    this->input = input;
    this->count = 0;
    this->generic_tokens = tokenize();
}

void Tokenizer::add_token(std::string &curr_arg, std::vector<Token> &tokens) {
    if (curr_arg.empty())
        return;
    Token token = Token(curr_arg);
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
                add_token(curr_arg, tokens);
                in_single_q = false;
            }
        } else if (in_double_q) {
            if (input[i] == '\\' && escape_backslashes) {
                curr_arg += input[++i];
                continue;
            }
            curr_arg += input[i];
            if (input[i] == '"') {
                add_token(curr_arg, tokens);
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
    this->command = tokens[0].arg;
    tokens.erase(tokens.begin());
    return tokens;
}

std::vector<Token> Tokenizer::get_tokens() { return generic_tokens; }

std::string Tokenizer::concat_args(bool add_space = false) {
    std::string res = "";
    for (auto tk : this->generic_tokens) {
        res += tk.arg;
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

std::string check_remove_quotes(std::string &token) {
    if (token.size() > 1 && token[token.size() - 1] == token[0] &&
        (token[0] == '\'' || token[0] == '"'))
        return token.substr(1, token.size() - 2);
    return token;
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
        std::vector<std::string> paths =
            split_args(std::string(directory_paths), file_sep);

        for (std::string path : paths) {

            if (!fs::exists(path))
                continue;

            for (const auto &entry : fs::directory_iterator(path)) {
                if (entry.path().stem() == filename) {
                    fs::path parent_path = entry.path().parent_path();
                    fs::path filename = entry.path().stem();
                    return (parent_path / filename).string();
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
        res += args[i].arg + sep;
    }
    res += args[n - 1].arg;
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

void custom_cat_cmd(std::vector<Token> args) {

    for (auto file_path : args) {

        try {

            if (file_path.arg == " " || file_path.arg == "")
                continue;

            std::string path = check_remove_quotes(file_path.arg);
            std::ifstream file;
            file.open(path);

            if (!file.is_open()) {
                std::cerr << "Error opening file : " << file_path.arg
                          << std::endl;
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
