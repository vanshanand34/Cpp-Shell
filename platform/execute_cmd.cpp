#include "execute_cmd.h"

int execute_cmd_linux(const char *cmd) {
#ifdef _WIN32
    return 0;
#else
    pid_t pid = fork();
    if (pid < 0) {
        std::cout << "Fork failed" << std::endl;
    } else if (pid == 0) {
        int output = execv(cmd, NULL);
        if (output == -1) {
            std::cout << "Error executing command : " << cmd << std::endl;
        }
        return 1;
    } else {
        waitpid(pid, NULL, 0);
        return 0;
    }
    return 0;
#endif
}

int execute_cmd(const char *cmd) {

#ifdef _WIN32
    return execute_cmd_windows(cmd);
#else
    return execute_cmd_linux(cmd);
#endif
}

int execute_cmd_windows(const char *cmd) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    int output = CreateProcess(NULL, // No module name (use command line)
                               (LPTSTR)(cmd), // Command line
                               NULL,          // Process handle not inheritable
                               NULL,          // Thread handle not inheritable
                               FALSE,         // Set handle inheritance to FALSE
                               0,             // No creation flags
                               NULL,          // Use parent's environment block
                               NULL,          // Use parent's starting directory
                               &si, // Pointer to STARTUPINFO structure
                               &pi  // Pointer to PROCESS_INFORMATION structure
    );

    // Start the child process.
    if (!output) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        return 0;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 1;
}