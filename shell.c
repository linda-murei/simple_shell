#include "shell.h"

#define BUFFER_SIZE 1024

/**
 * main - Entry point of the shell
 *
 * Return: Always 0
 */
int main(int argc, char **argv, char **envp)
{
    char *line;
    char **args;
    int status;

    (void)argc;
    (void)argv;

    while (1)
    {
        prompt();
        line = read_line();
        args = parse_line(line);
        status = execute_command(args, envp);

        handle_error(args[0], status);

        free(line);
        free(args);
    }

    return 0;
}

/**
 * prompt - Display the shell prompt
 */
void prompt(void)
{
    if (isatty(STDIN_FILENO))
        printf("$ ");
}

/**
 * read_line - Read a command line from the user
 *
 * Return: The command line entered by the user
 */
char *read_line(void)
{
    char *buffer = NULL;
    size_t bufsize = 0;
    ssize_t bytesRead;

    bytesRead = getline(&buffer, &bufsize, stdin);
    if (bytesRead == -1)
    {
        if (isatty(STDIN_FILENO))
            printf("\n");
        free(buffer);
        exit(EXIT_SUCCESS);
    }

    buffer[bytesRead - 1] = '\0'; /* Remove trailing newline */
    return buffer;
}

/**
 * count_tokens - Count the number of tokens in a line using delimiters
 * @line: The input line
 * @delimiters: The delimiters used to split the line
 *
 * Return: The number of tokens
 */
int count_tokens(char *line, char *delimiters)
{
    int count = 0;
    char *copy, *token;

    copy = strdup(line);
    if (copy == NULL)
    {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    token = strtok(copy, delimiters);
    while (token != NULL)
    {
        count++;
        token = strtok(NULL, delimiters);
    }

    free(copy);
    return count;
}

/**
 * split_tokens - Split the line into tokens using delimiters
 * @line: The input line
 * @delimiters: The delimiters used to split the line
 *
 * Return: An array of strings containing the tokens
 */
char **split_tokens(char *line, char *delimiters)
{
    char **tokens;
    char *copy, *token;
    int count, i = 0;

    count = count_tokens(line, delimiters);
    tokens = malloc(sizeof(char *) * (count + 1));
    if (tokens == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    copy = strdup(line);
    if (copy == NULL)
    {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    token = strtok(copy, delimiters);
    while (token != NULL)
    {
        tokens[i] = token;
        i++;
        token = strtok(NULL, delimiters);
    }
    tokens[i] = NULL;

    free(copy);
    return tokens;
}

/**
 * parse_line - Tokenize the command line into arguments
 * @line: The command line string
 *
 * Return: An array of strings containing the arguments
 */
char **parse_line(char *line)
{
    char **args;

    args = split_tokens(line, " \t\r\n");
    return args;
}



/**
 * execute_command - Execute a command with arguments
 * @args: An array of strings containing the command and arguments
 * @envp: The array of environment variables
 *
 * Return: The exit status of the command
 */
int execute_command(char **args, char **envp)
{
    pid_t pid;
    int status;
    char *path;
    char *path_copy;
    char *token;

    int is_background = 0;

    int num_args = 0;
    while (args[num_args] != NULL)
        num_args++;

    if (num_args > 0 && strcmp(args[num_args - 1], "&") == 0)
    {
        args[num_args - 1] = NULL;
        is_background = 1;
    }


    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (args[0][0] == '/')
        {
            if (execve(args[0], args, envp) == -1)
            {
                perror("execve");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            char *path_env = getenv("PATH");
            path_copy = strdup(path_env);

            if (path_copy == NULL)
            {
                perror("strdup");
                exit(EXIT_FAILURE);
            }

            token = strtok(path_copy, ":"); /* Move the declaration to the beginning of the block*/
            while (token != NULL)
            {
                path = malloc(strlen(token) + strlen(args[0]) + 2);
                if (path == NULL)
                {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                sprintf(path, "%s/%s", token, args[0]);

                if (access(path, X_OK) == 0)
                {
                    if (execve(path, args, envp) == -1)
                    {
                        perror("execve");
                        exit(EXIT_FAILURE);
                    }
                }

                free(path);
                token = strtok(NULL, ":");
            }

            fprintf(stderr, "%s: command not found\n", args[0]);
            free(path_copy);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (!is_background)
            waitpid(pid, &status, 0);

        return status;
    }

    return 0;
}

/**
 * check_builtin - Check if the command is a built-in command
 * @args: An array of strings containing the command and arguments
 * @envp: The array of environment variables
 *
 * Return: 1 if the command is a built-in, 0 otherwise
 */
int check_builtin(char **args, char **envp)
{
    if (args[0] == NULL)
        return 1;

    if (strcmp(args[0], "exit") == 0)
        return shell_exit(args);
    else if (strcmp(args[0], "env") == 0)
        return shell_env(envp);
    else if (strcmp(args[0], "setenv") == 0)
        return shell_setenv(args, envp);
    else if (strcmp(args[0], "unsetenv") == 0)
        return shell_unsetenv(args, envp);
    else if (strcmp(args[0], "cd") == 0)
        return shell_cd(args);

    return 0;
}

/**
 * handle_error - Handle command not found errors
 * @command: The command that was not found
 * @status: The exit status of the command
 */
void handle_error(char *command, int status)
{
    if (status == 127)
    {
        fprintf(stderr, "%s: command not found\n", command);
        fflush(stderr);
    }
}

/**
 * shell_exit - Handle the exit built-in command
 * @args: An array of strings containing the command and arguments
 *
 * Return: 1 to exit the shell
 */
int shell_exit(char **args)
{
    int status = 0;

    if (args[1] != NULL)
    {
        status = atoi(args[1]);
        if (status == 0 && args[1][0] != '0')
            status = 255;
    }

    exit(status);
}

/**
 * shell_env - Handle the env built-in command
 * @envp: The array of environment variables
 *
 * Return: 0 to continue the shell
 */
int shell_env(char **envp)
{
    int i = 0;

    while (envp[i] != NULL)
    {
        printf("%s\n", envp[i]);
        i++;
    }

    return 0;
}

/**
 * shell_setenv - Handle the setenv built-in command
 * @args: An array of strings containing the command and arguments
 * @envp: The array of environment variables
 *
 * Return: 0 to continue the shell
 */
int shell_setenv(char **args, char **envp)
{
    (void)envp;

    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "Usage: setenv variable value\n");
        return 0;
    }

    if (setenv(args[1], args[2], 1) == -1)
        perror("setenv");

    return 0;
}

/**
 * shell_unsetenv - Handle the unsetenv built-in command
 * @args: An array of strings containing the command and arguments
 * @envp: The array of environment variables
 *
 * Return: 0 to continue the shell
 */
int shell_unsetenv(char **args, char **envp)
{
    (void)envp;

    if (args[1] == NULL)
    {
        fprintf(stderr, "Usage: unsetenv variable\n");
        return 0;
    }

    if (unsetenv(args[1]) == -1)
        perror("unsetenv");

    return 0;
}

/**
 * shell_cd - Handle the cd built-in command
 * @args: An array of strings containing the command and arguments
 *
 * Return: 0 to continue the shell
 */
int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "Usage: cd directory\n");
        return 0;
    }

    if (chdir(args[1]) == -1)
        perror("cd");

    return 0;
}
