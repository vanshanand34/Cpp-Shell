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

pair<string, vector<string>> split_with_quotes(string str) {
    vector<string> arguments;
    string command = "";
    int n = str.size();
    string curr_arg = "";

    for (int i = 0; i < n; i++) {

        if (str[i] == '\'') {

            int closing_quote_idx = str.find('\'', i + 1);

            if (closing_quote_idx == string::npos) {
                curr_arg += str[i];
                continue;
            }

            curr_arg += str.substr(i, closing_quote_idx - i + 1);
            push_argument(curr_arg, str, arguments, closing_quote_idx, n);
            i = closing_quote_idx;

        } else if (str[i] == '"') {

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

    command = check_remove_quotes(arguments[0]);
    arguments.erase(arguments.begin());
    if (arguments[0] == " ")    arguments.erase(arguments.begin());

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
        vector<string> paths = split_args(directory_paths, ':');

        for (string path : paths) {
            for (const auto &entry : fs::directory_iterator(path)) {
                if (entry.path().stem() == input) {
                    string parent_path = entry.path().parent_path().string();
                    string filename = entry.path().stem().string();
                    return parent_path + "/" + filename;
                }
            }
        }
    } catch (const exception &ex) {
        cerr << ex.what() << endl;
    }
    return "";
}

string get_type(string command, char *directory_paths) {
    try {
        bool is_builtin = is_shell_builtin(command);

        if (is_builtin)
            return "builtin";

        string file_path = get_file_path(directory_paths, command);
        return file_path == "" ? "invalid" : file_path;

    } catch (const exception &e) {
        cerr << e.what() << endl;
    }

    return "invalid";
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