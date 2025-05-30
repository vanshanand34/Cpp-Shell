#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#define _pclose pclose
#define _popen popen
#endif

namespace fs = std::filesystem;

std::vector<std::string> split_args(std::string str, char delimiter) {
    std::stringstream ss(str);
    std::string item = "";
    std::vector<std::string> tokens;

    while (std::getline(ss, item, delimiter)) {
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

int shift_spaces(std::string &str, std::vector<std::string> &arguments,
                 int index, int n) {

    if (index == n || str[index] != ' ')
        return index;

    while (index < n && str[index] == ' ') {
        index++;
    }
    // check if there are more arguments, if yes add a space
    if (index < n) {
        arguments.push_back(" ");
    }

    return index;
}

int process_quotes(std::string &curr_token, std::string &str,
                   std::vector<std::string> &arguments, char quote, int i,
                   int n) {

    int closing_quote = -1;
    int index = i + 1;
    std::string temp_token = std::string(1, quote);
    
    // Preserve actual characters inside single quotes 
    // and special meaning inside double quotes (backslash)
    if (quote == '"') {
        while (index < n) {
            temp_token += str[index];
            if (str[index] == quote) {
                closing_quote = index;
                break;
            }
            if (str[index] == '\\') {
                if (index < n - 1) {
                    temp_token.pop_back();
                    temp_token += str[index + 1];
                }
                index++;
            }
            index++;
        }
    } else {
        while (index < n) {
            temp_token += str[index];
            if (str[index] == quote) {
                closing_quote = index;
                break;
            }
            index++;
        }
    }

    if (closing_quote == -1) {
        curr_token += str[i];
        return i;
    }

    if (curr_token != "")
        arguments.push_back(curr_token);

    arguments.push_back(temp_token);
    curr_token = "";
    i = shift_spaces(str, arguments, closing_quote + 1, n) - 1;
    return i;
}

std::pair<std::string, std::vector<std::string>>
split_with_quotes(std::string str) {
    std::vector<std::string> arguments;
    std::string curr_token = "";
    std::string command = "";
    int index = 0, n = str.size();

    while (index < n && str[index] != ' ')
        index++;

    command = str.substr(0, index);

    if (index == n)
        return {command, arguments};

    str = str.substr(index + 1);
    n = str.size();

    for (int i = 0; i < n; i++) {

        if (str[i] == '\'') {
            // single quote
            i = process_quotes(curr_token, str, arguments, '\'', i, n);

        } else if (str[i] == '"') {
            // double quote
            i = process_quotes(curr_token, str, arguments, '"', i, n);

        } else if (str[i] == ' ') {

            if (curr_token != "") {
                arguments.push_back(curr_token);
                curr_token = "";
            }
            i = shift_spaces(str, arguments, i, n) - 1;

        } else {
            if (str[i] == '\\') {
                if (i < n - 1)
                    curr_token += str[i + 1];
                i++;
                continue;
            }
            curr_token += str[i];
        }
    }

    if (curr_token != "")
        arguments.push_back(curr_token);

    return {command, arguments};
}

bool is_shell_builtin(std::string str) {
    std::string builtin_commands[] = {"pwd", "cd", "type", "exit", "echo"};
    for (std::string command : builtin_commands) {
        if (command == str)
            return true;
    }
    return false;
}

std::string get_file_path(char *directory_paths, std::string input) {
    try {
        std::string curr_path = "";
        std::vector<std::string> paths = split_args(directory_paths, ':');

        for (std::string path : paths) {
            for (const auto &entry : fs::directory_iterator(path)) {
                if (entry.path().stem() == input) {
                    std::string parent_path =
                        entry.path().parent_path().string();
                    std::string filename = entry.path().stem().string();
                    return parent_path + "/" + filename;
                }
            }
        }
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }
    return "";
}

std::string get_type(std::string command, char *directory_paths) {
    try {
        bool is_builtin = is_shell_builtin(command);

        if (is_builtin)
            return "builtin";

        std::string file_path = get_file_path(directory_paths, command);
        return file_path == "" ? "invalid" : file_path;

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return "invalid";
}

void exec_cat_cmd(std::vector<std::string> file_names) {
    for (std::string &file_name : file_names) {

        if (file_name == " " || file_name == "")
            continue;

        std::string cmd = "cat " + file_name;
        FILE *fp = _popen(cmd.c_str(), "r");

        if (fp == NULL) {
            std::cerr << "error opening file : " << file_name << std::endl;
            continue;
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            std::cout << buffer;
        }
        _pclose(fp);
    }
}

int main() {
    try {
        // Flush after every std::cout / std:cerr
        std::cout << std::unitbuf;
        std::cerr << std::unitbuf;

        char *directory_paths = getenv("PATH");
        char *home_path = getenv("HOME");
        std::string input = "input";

        while (true) {
            std::cout << "$ ";
            std::getline(std::cin, input);

            if (input.empty())
                return 0;

            auto [cmd, arguments] = split_with_quotes(input);

            if (cmd == "exit" && arguments.size() == 1 && arguments[0] == "0")
                return 0;

            if (cmd == "echo") {

                if (arguments.size() == 0)
                    continue;

                for (std::string token : arguments)
                    std::cout << check_remove_quotes(token);

                std::cout << std::endl;
                continue;
            }

            if (cmd == "type") {

                if (arguments.size() != 1)
                    continue;

                std::string cmd_type = get_type(arguments[0], directory_paths);

                if (cmd_type == "builtin")
                    std::cout << arguments[0] << " is a shell builtin"
                              << std::endl;

                else if (cmd_type == "invalid")
                    std::cout << arguments[0] << ": not found" << std::endl;

                else
                    std::cout << arguments[0] << " is " << cmd_type
                              << std::endl;

                continue;
            }

            if (cmd == "pwd") {
                std::cout << fs::current_path().string() << std::endl;
                continue;
            }

            if (cmd == "cd") {
                if (arguments.size() < 1)
                    continue;
                std::string destination_dir = input.substr(3);

                if (destination_dir == "~")
                    destination_dir = home_path;

                int ans = chdir(destination_dir.c_str());

                if (ans != 0) {
                    std::cout << "cd: " << destination_dir
                              << ": No such file or directory" << std::endl;
                }
                continue;
            }

            if (cmd == "cat") {
                exec_cat_cmd(arguments);
                continue;
            }

            // Check for executable files
            std::string file_path = get_file_path(directory_paths, cmd);

            if (file_path != "") {
                // Execute the file
                int output = system(input.c_str());
                continue;
            }

            std::cout << input << ": command not found" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
