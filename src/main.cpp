#include <iostream>
#include <string>

std::string strip(std::string &str, int n)
{
  int start = 0;
  int end = n - 1;
  while (start < n && str[start] == ' ')
    start++;

  if (start == n - 1)
    return "";

  while (end >= 0 && str[end] == ' ')
    end--;

  return str.substr(start, end - start + 1);
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
    input = strip(input, input.size());

    if (input.size() == 0)
      break;

    std::cout << input << ": command not found" << std::endl;
  }

  return 0;
}
