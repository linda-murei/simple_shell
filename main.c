#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

/**
 * main - Entry point for the shell program
 *
 * Return: Always returns EXIT_SUCCESS.
 */
int main(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("($) ");
        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while (status);

    return EXIT_SUCCESS;
}
