#ifndef SHELL_H
#define SHELL_H

/* Included headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

/* Function prototypes */
void prompt(void);
char *read_command(void);
char **parse_command(char *command);
int execute_command(char **args);
int check_builtin(char **args);
void handle_error(char *command, int status);

#endif /* SHELL_H */
