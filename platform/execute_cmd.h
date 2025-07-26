#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <windows.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

int execute_cmd(const char * str);
int execute_cmd_windows(const char * str);
int execute_cmd_linux(const char * str);