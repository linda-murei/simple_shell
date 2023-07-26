#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_TOKENS 64
#define DELIMITERS " \t\r\n\a"

/* Function Declarations */
void handle_signal(int signo);
char *read_input(void);
char **split_input(char *input);
int execute_command(char **args);
int is_builtin(char *command);
int handle_builtin(char **args);
void handle_variables(char **args);
void handle_comments(char *input);
void free_args(char **args);

/* Global Variable to keep track of program status */
int exit_shell = 0;

/* Main Shell Loop */
int main(void)
{
    char *input;
    char **args;
    int status;

    signal(SIGINT, handle_signal);

    while (1)
    {
        printf("($) ");
        input = read_input();
        if (input == NULL)
            break;

        handle_comments(input);
        args = split_input(input);
        handle_variables(args);

        status = execute_command(args);

        free_args(args);
        free(input);

        if (exit_shell)
            break;
    }

    return 0;
}

/* Signal Handler for SIGINT */
void handle_signal(int signo)
{
    (void)signo;
    putchar('\n');
    printf("($) ");
    fflush(stdout);
}

/* Read user input from stdin */
char *read_input(void)
{
    char *buffer = malloc(MAX_BUFFER_SIZE);
    if (!buffer)
    {
        perror("Error in memory allocation");
        exit(EXIT_FAILURE);
    }

    if (getline(&buffer, &MAX_BUFFER_SIZE, stdin) == -1)
    {
        free(buffer);
        return NULL;
    }

    return buffer;
}

/* Split input into tokens */
char **split_input(char *input)
{
    char **tokens = malloc(MAX_TOKENS * sizeof(char *));
    char *token;
    int index = 0;

    if (!tokens)
    {
        perror("Error in memory allocation");
        exit(EXIT_FAILURE);
    }

    token = strtok(input, DELIMITERS);
    while (token != NULL)
    {
        tokens[index++] = token;
        token = strtok(NULL, DELIMITERS);
    }

    tokens[index] = NULL;
    return tokens;
}

/* Execute the given command with arguments */
int execute_command(char **args)
{
    pid_t pid;
    int status;

    if (args[0] == NULL)
        return 1;

    if (is_builtin(args[0]))
        return handle_builtin(args);

    pid = fork();
    if (pid == 0)
    {
        /* Child process */
        if (execvp(args[0], args) == -1)
        {
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0)
    {
        /* Forking error */
        perror("Forking error");
    }
    else
    {
        /* Parent process */
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/* Check if a command is a built-in command */
int is_builtin(char *command)
{
    return (strcmp(command, "exit") == 0 || strcmp(command, "cd") == 0);
}

/* Handle built-in commands */
int handle_builtin(char **args)
{
    if (strcmp(args[0], "exit") == 0)
    {
        exit_shell = 1;
        return 1;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            fprintf(stderr, "Usage: cd <directory>\n");
        }
        else
        {
            if (chdir(args[1]) != 0)
                perror("cd");
        }
        return 1;
    }

    return 0; /* Not a built-in command */
}

/* Handle variable replacement (Not implemented in this example) */
void handle_variables(char **args)
{
    (void)args;
    // To be implemented, if desired.
}

/* Handle comments by removing them from the input */
void handle_comments(char *input)
{
    char *comment_ptr = strchr(input, '#');
    if (comment_ptr != NULL)
        *comment_ptr = '\0';
}

/* Free memory allocated for arguments */
void free_args(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        free(args[i]);
        i++;
    }
    free(args);
}
