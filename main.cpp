#include "utils.h"
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

int main() {
    try {
        // Flush after every std::cout / std:cerr
        std::cout << std::unitbuf;
        std::cerr << std::unitbuf;

        char *directory_paths = getenv("PATH");
        char *home_path = get_home_directory();
        std::string input;

        while (true) {
            std::cout << "$ ";
            std::getline(std::cin, input);

            if (input.empty())
                return 0;

            auto [cmd, echo_based_arguments] = split_with_quotes(input);
            std::vector<std::string> arguments =
                remove_spaces(echo_based_arguments);

            if (cmd == "exit" && arguments.size() == 1 && arguments[0] == "0")
                return 0;

            if (cmd == "echo") {
                for (std::string token : echo_based_arguments) {
                    std::cout << check_remove_quotes(token);
                }
                std::cout << std::endl;

            } else if (cmd == "type") {
                if (arguments.size() != 1) {
                    continue;
                }
                print_cmd_type(arguments[0], directory_paths);

            } else if (cmd == "pwd") {

                std::cout << fs::current_path().string() << std::endl;

            } else if (cmd == "cd") {

                if (arguments.size() < 1) {
                    continue;
                }
                std::string destination_dir = input.substr(3);

                if (destination_dir == "~") {
                    destination_dir = home_path;
                }
                int ans = chdir(destination_dir.c_str());

                if (ans != 0) {
                    std::cout << "cd: " << destination_dir
                              << ": No such file or directory" << std::endl;
                }

            } else if (cmd == "cat") {

                custom_cat_cmd(input.substr(4));

            } else {
                // Check for executable files
                std::string file_path = get_file_path(directory_paths, cmd);

                if (file_path != "") {
                    std::string processed_input =
                        process_exec_input(cmd, arguments);
                    int output = system(processed_input.c_str());
                    continue;
                }
                std::cout << input << ": command not found" << std::endl;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
    }

    return 0;
}
