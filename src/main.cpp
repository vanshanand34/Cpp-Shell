#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::string strip(std::string str, bool leading = true, bool trailing = true) {

  int n = str.size();
  int start = 0;
  int end = n - 1;

  if (leading) {
    while (start < n && str[start] == ' ')
      start++;
  }

  if (trailing) {
    while (end >= start && str[end] == ' ')
      end--;
  }

  if (start > end)
    return "";

  return str.substr(start, end - start + 1);
}

std::vector<std::string> split(std::string str, char delimiter) {
  std::stringstream ss(str);
  std::string item = "";
  std::vector<std::string> tokens;

  while (std::getline(ss, item, delimiter)) {
    tokens.push_back(item);
  }
  return tokens;
}

bool is_shell_builtin(std::string str) {
  std::string command_list[] = {"pwd", "type", "exit", "echo"};
  for (std::string command : command_list) {
    if (command == str)
      return true;
  }
  return false;
}

std::string get_file_path(char *directory_paths, std::string input) {
  try {
    std::string curr_path = "";
    std::vector<std::string> paths = split(directory_paths, ':');

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

int main() {
  try {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    char *directory_paths = getenv("PATH");
    std::string input = "input";

    while (true) {
      std::cout << "$ ";
      std::getline(std::cin, input);
      input = strip(input, true, false);
      std::vector<std::string> tokens = split(input, ' ');
      std::string cmd = tokens[0];

      if (input.empty())
        break;

      if (input == "exit 0")
        return 0;

      if (cmd == "echo") {
        if (tokens.size() == 1)
          continue;
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
          std::cout << cmd << ": not found" << std::endl;

        else
          std::cout << input.substr(5) << " is " << cmd_type << std::endl;

        continue;
      }

      if (cmd == "pwd") {
        std::cout << fs::current_path().string() << std::endl;
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
