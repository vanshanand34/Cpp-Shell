#include "utils.h"

int main() {
    try {
        // Flush after every std::cout / std:cerr
        std::cout << std::unitbuf;
        std::cerr << std::unitbuf;

        char *directory_paths = getenv("PATH");
        char *home_path = getenv("HOME");
        std::string input;

        while (true) {
            std::cout << "$ ";
            std::getline(std::cin, input);

            if (input.empty())
                return 0;

            auto [cmd, arguments] = split_with_quotes(input);

            if (cmd == "exit" && arguments.size() == 1 && arguments[0] == "0")
                return 0;

            if (cmd == "echo") {
                for (std::string token : arguments) {
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

                call_cat_cmd(input);

            } else {
                // Check for executable files
                std::string file_path = get_file_path(directory_paths, cmd);

                if (file_path != "") {
                    string processed_input = process_exec_input(cmd, arguments);
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
