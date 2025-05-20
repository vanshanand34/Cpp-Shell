#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

std::string strip(std::string str, bool leading = true, bool trailing = true)
{

  int n = str.size();
  int start = 0;
  int end = n - 1;

  if (leading)
  {
    while (start < n && str[start] == ' ')
      start++;
  }

  if (trailing)
  {
    while (end >= start && str[end] == ' ')
      end--;
  }

  if (start > end)
    return "";

  return str.substr(start, end - start + 1);
}

std::vector<std::string> split(std::string str, char delimiter)
{
  std::stringstream ss(str);
  std::string item = "";
  std::vector<std::string> tokens;

  while (std::getline(ss, item, delimiter))
  {
    tokens.push_back(item);
  }
  return tokens;
}

std::string commandType(std::string str)
{
  std::string command_list[] = {"type", "exit", "echo"};
  for (std::string command : command_list)
  {
    if (command == str)
      return " is a shell builtin";
  }
  return ": not found";
}

std::string findFileinDir(char *directory_paths, std::string input)
{
  std::string curr_path = "";
  std::vector<std::string> paths = split(directory_paths, ':');

  for (std::string path : paths)
  {
    std::cout << "path - " << path << std::endl;
    for (const auto &entry : fs::directory_iterator(path))
    {
      std::cout << "entry: " << entry.path().string() << std::endl;
      if (entry.path().stem() == input)
      {
        std::string parent_path = entry.path().parent_path().string();
        std::string filename = entry.path().stem().string();
        return parent_path + "\\" + filename;
      }
    }
  }
  return "";
}

int main()
{

  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  char *directory_paths = getenv("PATH");

  std::string input = "input";

  while (true)
  {
    std::cout << "$ ";
    std::getline(std::cin, input);
    input = strip(input, true, false);

    if (input.empty())
      break;

    if (input == "exit 0")
      return 0;

    if (input == "echo")
      continue;

    if (input.starts_with("echo"))
    {
      std::cout << input.substr(5) << std::endl;
      continue;
    }
    if (input.substr(0, 5) == "type ")
    {
      std::string command = input.substr(5);

      // First check if it is a shell builtin
      std::string command_type = commandType(command);
      if (command_type != ": not found")
      {
        std::cout << command << command_type << std::endl;
        continue;
      }

      std::string file_path = findFileinDir(directory_paths, input.substr(5));
      if (file_path != "")
      {
        std::cout << command << " is " << file_path << std::endl;
        continue;
      }

      std::cout << command << command_type << std::endl;
      continue;
    }

    std::cout << input << ": command not found" << std::endl;
  }

  return 0;
}
