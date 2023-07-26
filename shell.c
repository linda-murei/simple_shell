#include "shell.h"

#define BUFFER_SIZE 1024

/**
 * main - Entry point of the shell
 *
 * Return: Always 0
 */
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
		status = execute_command(args);

		handle_error(args[0], status);

		free(command);
		free(args);
	}

	return (0);
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
 * read_command - Read a command line from the user
 *
 * Return: The command line entered by the user
 */
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
	return (buffer);
}

/**
 * parse_command - Tokenize the command line into arguments
 * @command: The command line string
 *
 * Return: An array of strings containing the arguments
 */
char **parse_command(char *command)
{
	char **args = NULL;
	char *token;
	int i = 0;

	args = malloc(sizeof(char *) * 2);
	if (args == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	token = strtok(command, " ");
	while (token != NULL)
	{
		args[i] = token;
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

	return (args);
}

/**
 * execute_command - Execute a command with arguments
 * @args: An array of strings containing the command and arguments
 *
 * Return: The exit status of the command
 */
int execute_command(char **args)
{
	pid_t pid;
	int status;

	if (check_builtin(args))
		return (0);

	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		if (execve(args[0], args, NULL) == -1)
		{
			perror("execve");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		waitpid(pid, &status, 0);
		return (status);
	}

	return (0);
}

/**
 * check_builtin - Check if the command is a built-in command
 * @args: An array of strings containing the command and arguments
 *
 * Return: 1 if the command is a built-in, 0 otherwise
 */
int check_builtin(char **args)
{
	if (strcmp(args[0], "exit") == 0)
		exit(EXIT_SUCCESS);
	else if (strcmp(args[0], "env") == 0)
	{
		char *const env[] = {"env", NULL};

		if (execve("/usr/bin/env", env, NULL) == -1)
		{
			perror("execve");
			exit(EXIT_FAILURE);
		}
	}

	return (0);
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
