#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#define _pclose pclose
#define _popen popen
#endif

namespace fs = filesystem;

vector<string> split_args(string str, char delimiter) {
    stringstream ss(str);
    string item = "";
    vector<string> tokens;

    while (getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

string check_remove_quotes(string &token) {
    if (token.size() > 1 && token[token.size() - 1] == token[0] &&
        (token[0] == '\'' || token[0] == '"'))
        return token.substr(1, token.size() - 2);
    return token;
}

pair<int, string> process_double_q_str(string &raw_str, int index, int n) {

    string processed_str = "\"";

    for (int i = index + 1; i < n; i++) {

        if (i < n - 1 && raw_str[i] == '\\') {
            processed_str += raw_str[++i];
        } else if (raw_str[i] == '\'') {

            size_t single_q_end = raw_str.find('\'', i + 1);

            if (single_q_end == string::npos) {
                processed_str += raw_str[i];
                continue;
            }
            string single_q_str = raw_str.substr(i, single_q_end - i + 1);
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

void push_argument(string &curr_arg, string &str, vector<string> &arguments,
                   int &closing_index, int n) {

    arguments.push_back(curr_arg);

    if (closing_index < n - 1 && str[closing_index + 1] == ' ')
        arguments.push_back(" ");

    curr_arg = "";
}

string get_command(string &input) {
    string cmd = "";
    int closing_idx = 0;

    if (input[0] == ' ') {
        int idx = 0;
        while (idx < input.size() && input[idx] == ' ')
            idx++;
        input = input.substr(idx);
    }

    int n = input.size();

    if (input[0] == '\'' || input[0] == '"') {
        closing_idx = input.find(input[0], 1);
        if (closing_idx == string::npos) {
            cmd = input;
            input = "";
            return cmd;
        }

        cmd = input.substr(1, closing_idx - 1);
        // if (cmd.find(' ') != string::npos)
        input = input.substr(closing_idx + 1);
    } else {
        closing_idx = input.find(' ');
        if (closing_idx == string::npos) {
            cmd = input;
            input = "";
            return cmd;
        }
        cmd = input.substr(0, closing_idx);
        input = input.substr(closing_idx + 1);
    }

    return cmd;
}

pair<string, vector<string>> split_with_quotes(string str) {
    vector<string> arguments;
    string command = get_command(str);
    int n = str.size();
    string curr_arg = "";

    for (int i = 0; i < n; i++) {

        if (str[i] == '\'') {

            if (curr_arg.size() > 0)
                arguments.push_back(curr_arg);
            curr_arg = "";
            int closing_quote_idx = str.find('\'', i + 1);

            if (closing_quote_idx == string::npos) {
                curr_arg += str[i];
                continue;
            }

            curr_arg += str.substr(i, closing_quote_idx - i + 1);
            push_argument(curr_arg, str, arguments, closing_quote_idx, n);
            i = closing_quote_idx;

        } else if (str[i] == '"') {

            if (curr_arg.size() > 0)
                arguments.push_back(curr_arg);
            curr_arg = "";

            auto [closing_idx, processed_str] = process_double_q_str(str, i, n);

            if (closing_idx == -1) {
                curr_arg += str[i];
                continue;
            }

            push_argument(processed_str, str, arguments, closing_idx, n);

            i = closing_idx;
            curr_arg = "";

        } else if (str[i] == ' ') {

            if (curr_arg == "")
                continue;

            arguments.push_back(curr_arg);

            if (i < n - 1)
                arguments.push_back(" ");

            curr_arg = "";

        } else if (str[i] == '\\') {
            curr_arg += str[++i];
        } else {
            curr_arg += str[i];
        }
    }

    if (curr_arg != "")
        arguments.push_back(curr_arg);

    return {command, arguments};
}

bool is_shell_builtin(string str) {
    string builtin_commands[] = {"pwd", "cd", "type", "exit", "echo"};
    for (string command : builtin_commands) {
        if (command == str)
            return true;
    }
    return false;
}

string get_file_path(char *directory_paths, string input) {
    try {
        string curr_path = "";
        // cout << directory_paths << endl;
        vector<string> paths = split_args(directory_paths, ':');

        for (string path : paths) {

            if (!fs::exists(path))
                continue;

            for (const auto &entry : fs::directory_iterator(path)) {
                if (entry.path().stem() == input) {
                    fs::path parent_path = entry.path().parent_path();
                    fs::path filename = entry.path().stem();
                    return (parent_path / filename).string();
                }
            }
        }
    } catch (const exception &ex) {
        cerr << "Exception in get_file_path: " << ex.what() << endl;
    }
    return "";
}

string join(vector<string> args, string sep) {
    string res = "";
    int n = args.size();
    if (n == 0)
        return res;
    for (int i = 0; i < n - 1; i++) {
        res += args[i] + sep;
    }
    res += args[n - 1];
    return res;
}

void print_cmd_type(string command, char *directory_paths) {
    try {
        bool is_builtin = is_shell_builtin(command);

        if (is_builtin) {
            std::cout << command << " is a shell builtin" << std::endl;
            return;
        }

        string file_path = get_file_path(directory_paths, command);

        if (file_path == "")
            std::cout << command << ": not found" << std::endl;
        else
            std::cout << command << " is " << file_path << std::endl;

    } catch (const exception &e) {
        cerr << e.what() << endl;
    }
}

void exec_cat_cmd(vector<string> file_names) {
    for (string &file_name : file_names) {

        if (file_name == " " || file_name == "")
            continue;

        string cmd = "cat " + file_name;
        FILE *fp = _popen(cmd.c_str(), "r");

        if (fp == NULL) {
            cerr << "error opening file : " << file_name << endl;
            continue;
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            cout << buffer;
        }
        _pclose(fp);
    }
}

void call_cat_cmd(string input_cmd) {
    FILE *fp = _popen(input_cmd.c_str(), "r");
    if (fp == NULL) {
        cerr << "error opening file : " << input_cmd << endl;
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        cout << buffer;
    }
    _pclose(fp);
}

string process_exec_input(string cmd, vector<string> arguments) {
    string p_input = cmd;
    if (cmd.find(' ') != string::npos) {
        if (cmd.find('"') != string::npos)
            p_input = "'" + cmd + "'";
        else
            p_input = "\"" + cmd + "\"";
    }

    if (arguments.size() > 0)
        p_input += " " + join(arguments, " ");

    return p_input;
}