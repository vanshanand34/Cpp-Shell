#include "platform/execute_cmd.h"
#include "utils.h"
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif


#define CLEAR_SCREEN "\033[H\033[J"

namespace fs = std::filesystem;

int main() {
    try {
        // Flush after every std::cout / std:cerr
        std::cout << std::unitbuf;
        std::cerr << std::unitbuf;

        char *directory_paths = getenv("PATH");
        char *home_path = get_home_directory();
        std::vector<std::string> history;

        std::string input;
        std::cout << std::endl;
        while (true) {
            std::cout << "$ ";
            std::getline(std::cin, input);
            add_to_history(input, history);

            if (input.empty())
                return 0;

            Tokenizer t = Tokenizer(input);
            std::vector<Token> arguments = t.get_tokens();
            Token cmd = t.command;

            if (cmd.value == "exit" && arguments.size() == 1) {
                std::cout << std::endl;
                return arguments[0].value == "0" ? 0 : 1;
            }

            if (cmd.value == "echo") {
                for (auto tk : arguments) {
                    std::cout << tk.get_without_quotes();
                    if (tk.has_space)
                        std::cout << " ";
                }
                std::cout << std::endl;

            } else if (cmd.value == "type") {
                if (arguments.size() != 1) {
                    continue;
                }
                print_cmd_type(arguments[0].value, directory_paths);

            } else if (cmd.value == "pwd") {

                std::cout << fs::current_path().string() << std::endl;

            } else if (cmd.value == "cd") {

                if (arguments.size() < 1) {
                    continue;
                }

                std::string destination_dir = t.concat_args(false);

                if (destination_dir._Starts_with("~")) {
                    destination_dir = home_path + destination_dir.substr(1);
                }
                int ans = chdir(destination_dir.c_str());

                if (ans != 0) {
                    std::cout << "cd: " << destination_dir
                              << ": No such file or directory" << std::endl;
                }

            } else if (cmd.value == "cat") {

                custom_cat_cmd(t.get_cat_args());

            } else if (cmd.value == "history") {

                print_history(history);

            } else if (cmd.value == "clear") {
                
                std::cout << CLEAR_SCREEN;

            } else {
                // Check for executable files
                std::string file_path =
                    get_file_path(directory_paths, cmd.get_without_quotes());

                if (file_path != "") {
                    std::string processed_input =
                        process_exec_input(file_path, arguments);

                    int output = execute_cmd(processed_input.c_str());

                    if (output != 1) {
                        std::cerr << "Error executing command" << std::endl;
                    }
                    std::cout << std::endl;
                    continue;
                }
                std::cout << input << ": command not found" << std::endl;
            }
            std::cout << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
    }

    return 0;
}
