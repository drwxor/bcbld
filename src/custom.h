#ifndef CUSTOM_H
#define CUSTOM_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>

static int custom(int argc, char **argv, const char *dir_path) {
    if (argc < 3) {
        fprintf(stderr, "bcbld: missing custom command\n");
        return 1;
    }

    char *commands_dir = strconcat(dir_path, "/bcbld/commands/");
    if (!commands_dir)
        return 1;

    char *path = strconcat(commands_dir, argv[2]);

    free(commands_dir);

    if (!path)
        return 1;

    pid_t pid = fork();

    if (pid < 0) {
        perror("bcbld: fork");
        free(path);
        return 1;
    }

    if (pid == 0) {
        execv(path, &argv[2]);

        perror("bcbld: exec");
        free(path);
        _exit(127);
    }

    free(path);

    int status;

    if (waitpid(pid, &status, 0) < 0) {
        perror("bcbld: waitpid");
        return 1;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);

    return 1;
};

#endif
