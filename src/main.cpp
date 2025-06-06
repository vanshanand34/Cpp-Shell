#include "utils.h"

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

            auto [cmd, arguments] = split_with_quotes(input);
            // cout << "commmand: " << cmd << " " << cmd.size() << endl;
            // for (string s : arguments) {
            //     cout << s << endl;
            // }

            if (cmd == "exit" && arguments.size() == 1 && arguments[0] == "0")
                return 0;

            if (cmd == "echo") {

                if (arguments.size() == 0)
                    continue;

                for (std::string token : arguments)
                    std::cout << check_remove_quotes(token);

                std::cout << std::endl;
                continue;
            }

            if (cmd == "type") {

                if (arguments.size() != 1)
                    continue;

                std::string cmd_type = get_type(arguments[0], directory_paths);

                if (cmd_type == "builtin")
                    std::cout << arguments[0] << " is a shell builtin"
                              << std::endl;

                else if (cmd_type == "invalid")
                    std::cout << arguments[0] << ": not found" << std::endl;

                else
                    std::cout << arguments[0] << " is " << cmd_type
                              << std::endl;

                continue;
            }

            if (cmd == "pwd") {
                std::cout << fs::current_path().string() << std::endl;
                continue;
            }

            if (cmd == "cd") {
                if (arguments.size() < 1)
                    continue;
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
                call_cat_cmd(input);
                continue;
            }

            // Check for executable files
            std::string file_path = get_file_path(directory_paths, cmd);

            if (file_path != "") {
                // Execute the file
                string processed_input =
                    cmd.find(' ') != string::npos ? "\"" + cmd + "\"" : cmd;
                if (arguments.size() > 0)
                    processed_input =
                        processed_input + " " +
                        join(arguments,
                             " "); // In case command in inside quotes
                // cout << processed_input << endl;
                int output = system(processed_input.c_str());
                // if (output != 0)
                //     cout << "Error executing file" << endl;
                continue;
            }

            std::cout << input << ": command not found" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
    }

    return 0;
}
