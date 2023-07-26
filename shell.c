#include "shell.h"

#define BUFFER_SIZE 1024

int main(void)
{
    char *command;
    char **args;
    int status;

    while (1)
    {
        prompt();
        command = read_command();
        args = parse_command(command);

        /* Check if the command is empty (user pressed enter)*/
        if (args[0] == NULL)
        {
            free(command);
            free(args);
            continue;
        }

        status = execute_command(args);

        handle_error(args[0], status);

        free(command);
        free(args);
    }

    return 0;
}

void prompt(void)
{
    if (isatty(STDIN_FILENO))
        printf("$ ");
}

char *read_command(void)
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

char **parse_command(char *command)
{
    char **args = NULL;
    char *token;
    int i = 0;

    args = malloc(sizeof(char *));
    if (args == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    token = strtok(command, " ");
    while (token != NULL)
    {
        args[i] = strdup(token); /* Use strdup to duplicate the token*/
        i++;
        args = realloc(args, sizeof(char *) * (i + 1));
        if (args == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    return args;
}

int execute_command(char **args)
{
    pid_t pid;
    int status;

    if (check_builtin(args))
        return 0;

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        waitpid(pid, &status, 0);
        return status;
    }

    return 0;
}

int check_builtin(char **args)
{
    if (strcmp(args[0], "exit") == 0)
    {
        handle_exit(args);
        return 1;
    }
    else if (strcmp(args[0], "env") == 0)
    {
        handle_env();
        return 1;
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        handle_cd(args);
        return 1;
    }
    else if (strcmp(args[0], "setenv") == 0)
    {
        handle_setenv(args);
        return 1;
    }
    else if (strcmp(args[0], "unsetenv") == 0)
    {
        handle_unsetenv(args);
        return 1;
    }
    else if (strcmp(args[0], "alias") == 0)
    {
        handle_alias(args);
        return 1;
    }

    return 0;
}

void handle_error(char *command, int status)
{
    if (status == 127)
    {
        fprintf(stderr, "%s: command not found\n", command);
        fflush(stderr);
    }
}

void handle_exit(char **args)
{
    /* If there's an argument, attempt to convert it to an integer*/
    if (args[1] != NULL)
    {
        int exit_status = atoi(args[1]);
        exit(exit_status);
    }
    else
    {
        exit(EXIT_SUCCESS);
    }
}

void handle_env(void)
{
    extern char **environ;
    char **env;
    for (env = environ; *env != NULL; env++)
    {
        printf("%s\n", *env);
    }
}

void handle_cd(char **args)
{
    char *dir = args[1];

    if (dir == NULL)
        dir = getenv("HOME");

    if (dir == NULL)
    {
        fprintf(stderr, "cd: no $HOME variable set\n");
        return;
    }

    if (chdir(dir) != 0)
    {
        perror("cd");
    }
    else
    {
        /* Update the PWD environment variable*/
        char cwd[BUFFER_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            setenv("PWD", cwd, 1);
        }
        else
        {
            perror("getcwd");
        }
    }
}

void handle_setenv(char **args)
{
    char *name = args[1];
    char *value = args[2];

    if (name == NULL || value == NULL)
    {
        fprintf(stderr, "Usage: setenv VARIABLE VALUE\n");
        return;
    }

    if (setenv(name, value, 1) != 0)
    {
        perror("setenv");
    }
}

void handle_unsetenv(char **args)
{
    char *name = args[1];

    if (name == NULL)
    {
        fprintf(stderr, "Usage: unsetenv VARIABLE\n");
        return;
    }

    if (unsetenv(name) != 0)
    {
        perror("unsetenv");
    }
}

void handle_alias(char **args)
{
    if (args[1] == NULL)
    {
        /* Print all aliases*/
        extern char **environ;
        char **env;
        for (env = environ; *env != NULL; env++)
        {
            if (strncmp(*env, "alias ", 6) == 0)
            {
                printf("%s\n", *env + 6);
            }
        }
    }
    else
    {
        /* Define or update an alias*/
        char *name = args[1];
        char *value = args[2];

        if (name == NULL)
        {
            fprintf(stderr, "Usage: alias [name[='value'] ...]\n");
            return;
        }

        /* If there's no value provided, print the alias*/
        if (value == NULL)
        {
            extern char **environ;
            char **env;
            for (env = environ; *env != NULL; env++)
            {
                if (strncmp(*env, "alias ", 6) == 0 && strncmp(*env + 6, name, strlen(name)) == 0)
                {
                    printf("%s\n", *env + 6);
                    break;
                }
            }
        }
        else
        {
            char *alias = malloc(strlen(name) + strlen(value) + 8);
            if (alias == NULL)
            {
                perror("malloc");
                return;
            }
            sprintf(alias, "alias %s='%s'", name, value);
            putenv(alias); /* Add the alias to the environment*/
        }
    }
}
