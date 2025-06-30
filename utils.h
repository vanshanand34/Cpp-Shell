#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <sstream>



std::vector<std::string> split_args(std::string str, char delimiter);

std::string check_remove_quotes(std::string &token);

std::pair<int, std::string> process_double_q_str(std::string &raw_str,
                                                 int index, int n);

void push_argument(std::string &curr_arg, std::string &str,
                   std::vector<std::string> &arguments, int &closing_index,
                   int n);

std::string get_command(std::string &input);

std::pair<std::string, std::vector<std::string>>
split_with_quotes(std::string str);

bool is_shell_builtin(std::string str);

std::string get_file_path(char *directory_paths, std::string filename);

std::string join(std::vector<std::string> args, std::string sep);

void print_cmd_type(std::string command, char *directory_paths);

std::vector<std::string> split_cat_args(std::string file_path_str);

void custom_cat_cmd(std::string file_path_str);

std::vector<std::string> remove_spaces(std::vector<std::string> args);

std::string process_exec_input(std::string cmd,
                               std::vector<std::string> arguments);

char* get_home_directory();

#endif