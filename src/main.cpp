#include <iostream>
#include <string>

std::string strip(std::string str, bool leading = true, bool trailing = true)
{
  // If leading is false, it works as an rstrip function
  // If trailing is false, it works as an lstrip function
  // And if both true, then is works as a general strip function

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

int main()
{

  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

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

    if (input.substr(0, 5) == "echo ")
    {
      std::cout << input.substr(5) << std::endl;
      continue;
    }
    if (input.substr(0, 5) == "type ")
    {
      std::string command_type = commandType(input.substr(5));
      std::cout << input.substr(5) << command_type << std::endl;
      continue;
    }

    std::cout << input << ": command not found" << std::endl;
  }

  return 0;
}
