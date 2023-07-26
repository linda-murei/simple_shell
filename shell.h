#ifndef SHELL_H
#define SHELL_H

/**
 * shell_read_line - Read a line of input from the user.
 *
 * Return: The input line as a string.
 */
char *shell_read_line(void);

/**
 * shell_split_line - Split a line into tokens.
 * @line: The input line to be tokenized.
 *
 * Return: An array of pointers to the tokens.
 */
char **shell_split_line(char *line);

/**
 * shell_execute - Execute the command and its arguments.
 * @args: An array of pointers to the command and its arguments.
 *
 * Return: 1 if the shell should continue, 0 if the shell should exit.
 */
int shell_execute(char **args);

#endif /* SHELL_H */
