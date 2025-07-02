#ifndef UTILS_H
#define UTILS_H

#include <sstream>
#include <string>
#include <vector>

class Token {
  public:
    std::string arg;
    bool has_space;

    Token(std::string arg, bool has_space);
    void set_has_space();
};

// Full definition of Tokenizer
class Tokenizer {
    std::vector<Token> generic_tokens;
    std::string input;
    int count;

    std::vector<Token> tokenize(bool escape_backslash = true);
    void add_token(std::string &curr_arg, std::vector<Token> &tokens);

  public:
    std::string command;
    Tokenizer(std::string input);
    std::vector<Token> get_tokens();
    std::vector<Token> get_cat_args();
    std::string concat_args(bool add_space);
};

std::vector<std::string> split_args(std::string str, char delimiter);

std::string check_remove_quotes(std::string &token);

std::pair<int, std::string> process_double_q_str(std::string &raw_str,
                                                 int index, int n);

void push_argument(std::string &curr_arg, std::string &str,
                   std::vector<std::string> &arguments, int &closing_index,
                   int n);


bool is_shell_builtin(std::string str);

std::string get_file_path(char *directory_paths, std::string filename);

std::string join(std::vector<std::string> args, std::string sep);

void print_cmd_type(std::string command, char *directory_paths);

void custom_cat_cmd(std::vector<Token> arguments);

std::vector<std::string> remove_spaces(std::vector<std::string> args);

std::string process_exec_input(std::string cmd, std::vector<Token> arguments);

char *get_home_directory();

#endif