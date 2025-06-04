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

int shift_spaces(string &str, vector<string> &arguments, int index, int n) {

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

vector<bool> check_matching_quotes(string command_str, int n) {
    stack<int> double_quotes;
    stack<int> single_quotes;

    vector<bool> quotes_match(n, false);

    for (int i = 0; i < n; i++) {
        if (command_str[i] == '\'') {
            if (single_quotes.empty()) {
                single_quotes.push(i);
            } else {
                quotes_match[single_quotes.top()] = true;
                while (!double_quotes.empty() &&
                       double_quotes.top() > single_quotes.top())
                    double_quotes.pop();
                single_quotes.pop();
            }
        } else if (command_str[i] == '"') {
            if (double_quotes.empty()) {
                double_quotes.push(i);
            } else {
                quotes_match[double_quotes.top()] = true;
                while (!single_quotes.empty() &&
                       single_quotes.top() > double_quotes.top())
                    single_quotes.pop();
                double_quotes.pop();
            }
        } else if (command_str[i] == '\\') {
            if (!double_quotes.empty()) {
                i++;
            }
        }
    }
    return quotes_match;
}

pair<int, string> process_double_quotes(string &raw_str, int index, int n) {

    string processed_str = "\"";

    for (int i = index + 1; i < n; i++) {

        if (i < n - 1 && raw_str[i] == '\\') {
            processed_str += raw_str[i + 1];
            i++;
        } else if (raw_str[i] == '\'') {
            size_t closing_nested_single = raw_str.find('\'', i + 1);
            if (closing_nested_single != string::npos) {
                string nested_single_quoted_str =
                    raw_str.substr(i, closing_nested_single - i + 1);
                processed_str += nested_single_quoted_str;
                i = closing_nested_single;
            } else {
                processed_str += raw_str[i];
            }
        } else if (raw_str[i] == '"') {
            return {i, processed_str + '"'};
        } else {
            processed_str += raw_str[i];
        }
    }
    return {-1, ""};
}

pair<string, vector<string>> split_with_quotes(string str) {
    vector<string> arguments;
    string command = "";
    int index = 0, n = str.size();

    while (index < n && str[index] != ' ')
        index++;

    command = str.substr(0, index);

    if (index == n)
        return {command, arguments};

    str = str.substr(index + 1);
    n = str.size();
    string curr_arg = "";

    for (int i = 0; i < n; i++) {
        if (str[i] == '\'') {
            size_t closing_quote_index = str.find('\'', i + 1);
            if (closing_quote_index != string::npos) {
                curr_arg += str.substr(i, closing_quote_index - i + 1);
                arguments.push_back(curr_arg);
                if (closing_quote_index < n - 1 &&
                    str[closing_quote_index + 1] == ' ')
                    arguments.push_back(" ");
                i = closing_quote_index;
                curr_arg = "";
            } else {
                curr_arg += str[i];
            }
        } else if (str[i] == '"') {
            auto [closing_index, processed_str] =
                process_double_quotes(str, i, n);
            if (closing_index != -1) {
                arguments.push_back(processed_str);
                if (i < n - 1 && str[i + 1] == ' ')
                    arguments.push_back(" ");
                i = closing_index;
                curr_arg = "";
            } else {
                curr_arg += str[i];
            }
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
    cout << endl;
    _pclose(fp);
}