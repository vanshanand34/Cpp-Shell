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

  if (token.size() > 1 && token[0] == '\'' && token[token.size() - 1] == '\'')
    return token.substr(1, token.size() - 2);
  return token;
}

std::vector<std::string> split_with_quotes(std::string str) {
  std::vector<std::string> tokens;
  std::string curr_token = "";

  for (int i = 0; i < str.size(); i++) {
    if ((str[i] == ' ' && curr_token[0] != '\'')) {

      if (curr_token != "")
        tokens.push_back(curr_token);

      curr_token = "";
      continue;
    }
    if (str[i] == '\'' && curr_token[0] == '\'') {

      // curr_token = curr_token.substr(1, curr_token.size() - 1);
      tokens.push_back(curr_token + '\'');
      curr_token = "";
      continue;
    }

    curr_token += str[i];
  }

  if (curr_token != "")
    tokens.push_back(curr_token);

  return tokens;
}

bool is_shell_builtin(std::string str) {
  std::string command_list[] = {"pwd", "cd", "type", "exit", "echo", "cat"};
  for (std::string command : command_list) {
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
          std::string parent_path = entry.path().parent_path().string();
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
    // First check if command is a shell builtin
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

    if (file_name == "cat")
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


      std::vector<std::string> tokens = split_with_quotes(input);
      std::string cmd = tokens[0];

      if (input == "exit 0")
        return 0;

      if (cmd == "echo") {

        if (tokens.size() == 1)
          continue;

        if (input.contains('\'')) {

          for (std::string token : tokens) {
            if (token == "echo")
              continue;
            std::cout << check_remove_quotes(token) << " ";
          }
          std::cout << std::endl;
          continue;
        }
        std::cout << input.substr(5) << std::endl;
        continue;
      }

      if (cmd == "type") {

        if (tokens.size() == 1)
          continue;

        std::string cmd_type = get_type(input.substr(5), directory_paths);

        if (cmd_type == "builtin")
          std::cout << input.substr(5) << " is a shell builtin" << std::endl;

        else if (cmd_type == "invalid")
          std::cout << input.substr(5) << ": not found" << std::endl;

        else
          std::cout << input.substr(5) << " is " << cmd_type << std::endl;

        continue;
      }

      if (cmd == "pwd") {
        std::cout << fs::current_path().string() << std::endl;
        continue;
      }

      if (cmd == "cd") {
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
        exec_cat_cmd(tokens);
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
