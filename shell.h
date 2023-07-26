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
char *read_line(void);
char **parse_line(char *line);
int execute_command(char **args, char **envp);
int check_builtin(char **args, char **envp);
void handle_error(char *command, int status);
int count_tokens(char *line, char *delimiters);
char **split_tokens(char *line, char *delimiters);

/* Builtin command prototypes */
int shell_exit(char **args);
int shell_env(char **envp);
int shell_setenv(char **args, char **envp);
int shell_unsetenv(char **args, char **envp);
int shell_cd(char **args);

#endif /* SHELL_H */
