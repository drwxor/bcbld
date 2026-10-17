#ifndef CUSTOM_H
#define CUSTOM_H

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"

#ifdef _WIN32

#include "proc.h"

static int custom_is_file(const char *path) {
    DWORD attributes = GetFileAttributesA(path);

    return attributes != INVALID_FILE_ATTRIBUTES &&
        !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

/*
 * Finds the command file, trying the name as given first and then the
 * usual executable extensions, so `bcbld cmd run` finds run.bat or run.exe.
 */
static char *custom_resolve(const char *path) {
    static const char *extensions[] = { "", ".exe", ".com", ".bat", ".cmd" };

    for (size_t i = 0; i < sizeof(extensions) / sizeof(*extensions); i++) {
        char *candidate = strconcat(path, extensions[i]);

        if (!candidate)
            return NULL;

        if (custom_is_file(candidate)) {
            for (char *p = candidate; *p; p++) {
                if (*p == '/')
                    *p = '\\';
            }

            return candidate;
        }

        free(candidate);
    }

    return NULL;
}

static int custom_is_batch(const char *path) {
    const char *extension = strrchr(path, '.');

    return extension &&
        (_stricmp(extension, ".bat") == 0 || _stricmp(extension, ".cmd") == 0);
}

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

    char *resolved = custom_resolve(path);

    if (!resolved) {
        fprintf(stderr, "bcbld: command not found: %s\n", path);
        free(path);
        return 1;
    }

    free(path);

    char **args = malloc((size_t)(argc - 1) * sizeof(*args));

    if (!args) {
        free(resolved);
        return 1;
    }

    args[0] = resolved;

    for (int i = 3; i < argc; i++)
        args[i - 2] = argv[i];

    args[argc - 2] = NULL;

    char *command_line = proc_command_line(args);

    free(args);

    if (!command_line) {
        free(resolved);
        return 1;
    }

    const char *application = resolved;

    if (custom_is_batch(resolved)) {
        /* batch files run through cmd.exe; /s strips the outer quotes */
        char *tmp = strconcat("cmd.exe /d /s /c \"", command_line);

        free(command_line);

        command_line = tmp ? strconcat(tmp, "\"") : NULL;

        free(tmp);

        if (!command_line) {
            free(resolved);
            return 1;
        }

        application = getenv("COMSPEC");
    }

    proc_handle_t process = proc_create(application, command_line);

    free(command_line);

    if (process == INVALID_PROC_HANDLE) {
        fprintf(
            stderr,
            "bcbld: failed to run %s (error %lu)\n",
            resolved,
            GetLastError()
        );

        free(resolved);
        return 1;
    }

    free(resolved);

    int exit_code;

    if (proc_wait(process, &exit_code) < 0) {
        fprintf(stderr, "bcbld: failed to wait for command\n");
        proc_close(process);
        return 1;
    }

    proc_close(process);

    return exit_code;
}

#else

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

#endif
