#ifndef UTILS_H
#define UTILS_H

#include <sstream>
#include <string>
#include <vector>

class Token {
  public:
    std::string value;
    bool has_space;
    int quote_type;

    Token() : value(""), has_space(false), quote_type(0) {}
    Token(std::string arg, int quote_type = 0);
    void set_has_space();
    std::string get_without_quotes();
};

// Full definition of Tokenizer
class Tokenizer {
    std::vector<Token> generic_tokens;
    std::string input;
    int count;

    std::vector<Token> tokenize(bool escape_backslash = true);
    void add_token(std::string &curr_arg, std::vector<Token> &tokens,
                   int quote_type = 0);

  public:
    Token command;
    Tokenizer(std::string input);
    std::vector<Token> get_tokens();
    std::vector<Token> get_cat_args();
    std::string concat_args(bool add_space);
};

std::vector<std::string> split_args(std::string str, char delimiter);

bool is_shell_builtin(std::string str);

std::string get_file_path(char *directory_paths, std::string filename);

std::string join(std::vector<std::string> args, std::string sep);

void print_cmd_type(std::string command, char *directory_paths);

void custom_cat_cmd(std::vector<Token> arguments);

std::string process_exec_input(std::string cmd, std::vector<Token> arguments);

char *get_home_directory();

void add_to_history(std::string command, std::vector<std::string> &history);

void print_history(std::vector<std::string> history);

void clear_screen();

#endif