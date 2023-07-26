#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "shell.h"

#define SHELL_BUFSIZE 1024
#define SHELL_TOKEN_DELIMITER " \t\r\n\a"

/**
 * shell_read_line - Read a line of input from the user.
 *
 * Return: The input line as a string.
 */
char *shell_read_line(void)
{
    static char buffer[SHELL_BUFSIZE];
    int position = 0;
    int c;

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= SHELL_BUFSIZE) {
            fprintf(stderr, "shell: command too long\n");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * shell_split_line - Split a line into tokens.
 * @line: The input line to be tokenized.
 *
 * Return: An array of pointers to the tokens.
 */
char **shell_split_line(char *line)
{
    int bufsize = SHELL_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        perror("shell: allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHELL_TOKEN_DELIMITER);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += SHELL_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                perror("shell: allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHELL_TOKEN_DELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * shell_execute - Execute the command and its arguments.
 * @args: An array of pointers to the command and its arguments.
 *
 * Return: 1 if the shell should continue, 0 if the shell should exit.
 */
int shell_execute(char **args)
{
    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) {
            int exit_status = atoi(args[1]);
            exit(exit_status);
        } else {
            exit(EXIT_SUCCESS);
        }
    } else if (strcmp(args[0], "env") == 0) {
        char *const *env = environ;
        while (*env) {
            printf("%s\n", *env);
            env++;
        }
        return 1;
    } else if (strcmp(args[0], "setenv") == 0) {
        if (args[1] == NULL || args[2] == NULL) {
            fprintf(stderr, "Usage: setenv VARIABLE VALUE\n");
            return 1;
        }
        if (setenv(args[1], args[2], 1) != 0) {
            perror("shell");
        }
        return 1;
    } else if (strcmp(args[0], "unsetenv") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Usage: unsetenv VARIABLE\n");
            return 1;
        }
        if (unsetenv(args[1]) != 0) {
            perror("shell");
        }
        return 1;
    } else if (strcmp(args[0], "cd") == 0) {
        char *path = args[1];
        if (path == NULL) {
            path = getenv("HOME");
            if (path == NULL) {
                fprintf(stderr, "shell: cd: HOME not set\n");
                return 1;
            }
        }
        if (chdir(path) != 0) {
            perror("shell");
        } else {
            char cwd[SHELL_BUFSIZE];
            if (getcwd(cwd, SHELL_BUFSIZE) != NULL) {
                setenv("PWD", cwd, 1);
            } else {
                perror("shell");
            }
        }
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "%s: command not found\n", args[0]);
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("shell");
    } else {
        // Parent process
        wait(NULL);
    }

    return 1;
}
