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
        if (!in_single_q && input[i] == '\\' && i == n - 1 && escape_backslashes)
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

std::vector<Token> Tokenizer::get_cat_args() {
    return tokenize(false);
}

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

std::pair<int, std::string> process_double_q_str(std::string &raw_str,
                                                 int index, int n) {

    std::string processed_str = "\"";

    for (int i = index + 1; i < n; i++) {

        if (i < n - 1 && raw_str[i] == '\\') {
            processed_str += raw_str[++i];
        } else if (raw_str[i] == '\'') {

            size_t single_q_end = raw_str.find('\'', i + 1);

            if (single_q_end == std::string::npos) {
                processed_str += raw_str[i];
                continue;
            }
            std::string single_q_str = raw_str.substr(i, single_q_end - i + 1);
            processed_str += single_q_str;
            i = single_q_end;

        } else if (raw_str[i] == '"') {
            return {i, processed_str + '"'};
        } else {
            processed_str += raw_str[i];
        }
    }
    return {-1, ""};
}

void push_argument(std::string &curr_arg, std::string &str,
                   std::vector<std::string> &arguments, int &closing_index,
                   int n) {

    arguments.push_back(curr_arg);

    if (closing_index < n - 1 && str[closing_index + 1] == ' ')
        arguments.push_back(" ");

    curr_arg = "";
}

// std::string get_command(std::string &input) {
//     std::string cmd = "";
//     int closing_idx = 0;

//     if (input[0] == ' ') {
//         int idx = 0;
//         while (idx < input.size() && input[idx] == ' ')
//             idx++;
//         input = input.substr(idx);
//     }

//     int n = input.size();

//     if (input[0] == '\'' || input[0] == '"') {
//         closing_idx = input.find(input[0], 1);
//         if (closing_idx == std::string::npos) {
//             cmd = input;
//             input = "";
//             return cmd;
//         }

//         cmd = input.substr(1, closing_idx - 1);
//         // if (cmd.find(' ') != std::string::npos)
//         input = input.substr(closing_idx + 1);
//     } else {
//         closing_idx = input.find(' ');
//         if (closing_idx == std::string::npos) {
//             cmd = input;
//             input = "";
//             return cmd;
//         }
//         cmd = input.substr(0, closing_idx);
//         input = input.substr(closing_idx + 1);
//     }

//     return cmd;
// }

// std::pair<std::string, std::vector<std::string>>
// split_with_quotes(std::string str) {
//     std::vector<std::string> arguments;
//     std::string command = get_command(str);
//     int n = str.size();
//     std::string curr_arg = "";

//     for (int i = 0; i < n; i++) {

//         if (str[i] == '\'') {

//             if (curr_arg.size() > 0)
//                 arguments.push_back(curr_arg);
//             curr_arg = "";
//             int closing_quote_idx = str.find('\'', i + 1);

//             if (closing_quote_idx == std::string::npos) {
//                 curr_arg += str[i];
//                 continue;
//             }

//             curr_arg += str.substr(i, closing_quote_idx - i + 1);
//             push_argument(curr_arg, str, arguments, closing_quote_idx, n);
//             i = closing_quote_idx;

//         } else if (str[i] == '"') {

//             if (curr_arg.size() > 0)
//                 arguments.push_back(curr_arg);
//             curr_arg = "";

//             auto [closing_idx, processed_str] = process_double_q_str(str, i,
//             n);

//             if (closing_idx == -1) {
//                 curr_arg += str[i];
//                 continue;
//             }

//             push_argument(processed_str, str, arguments, closing_idx, n);

//             i = closing_idx;
//             curr_arg = "";

//         } else if (str[i] == ' ') {

//             if (curr_arg == "")
//                 continue;

//             arguments.push_back(curr_arg);

//             if (i < n - 1)
//                 arguments.push_back(" ");

//             curr_arg = "";

//         } else if (str[i] == '\\') {
//             curr_arg += str[++i];
//         } else {
//             curr_arg += str[i];
//         }
//     }

//     if (curr_arg != "")
//         arguments.push_back(curr_arg);

//     return {command, arguments};
// }

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

    // std::vector<std::string> file_paths = split_cat_args(file_path_str);

    for (auto file_path : args) {

        if (file_path.arg == " " || file_path.arg == "")
            continue;

        std::string path_without_quotes = check_remove_quotes(file_path.arg);
        std::ifstream file;
        file.open(path_without_quotes);
        std::string chunk;

        if (!file.is_open()) {
            std::cerr << "error opening file : " << file_path.arg << std::endl;
            continue;
        }

        std::cout << file.rdbuf();
        file.close();
    }
}

std::vector<std::string> remove_spaces(std::vector<std::string> args) {
    std::vector<std::string> args_without_spaces;
    for (std::string arg : args) {
        if (arg != " " || arg != "")
            args_without_spaces.push_back(arg);
    }
    return args_without_spaces;
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
