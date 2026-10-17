#ifndef CBLD_PROC_H
#define CBLD_PROC_H

#include <stdio.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <string.h>

typedef HANDLE proc_handle_t;
#define INVALID_PROC_HANDLE ((HANDLE)NULL)

/*
 * Appends one argument to a command line, quoted so that it is parsed back
 * into the same argv entry by CommandLineToArgvW and the MSVC runtime.
 */
static size_t proc_quote_arg(char *out, const char *arg) {
    size_t n = 0;

    if (arg[0] != '\0' && !strpbrk(arg, " \t\n\v\"")) {
        size_t len = strlen(arg);

        if (out)
            memcpy(out, arg, len);

        return len;
    }

    if (out)
        out[n] = '"';
    n++;

    for (const char *p = arg; ; p++) {
        size_t backslashes = 0;

        while (*p == '\\') {
            backslashes++;
            p++;
        }

        if (*p == '\0')
            backslashes *= 2;
        else if (*p == '"')
            backslashes = backslashes * 2 + 1;

        for (size_t i = 0; i < backslashes; i++) {
            if (out)
                out[n] = '\\';
            n++;
        }

        if (*p == '\0')
            break;

        if (out)
            out[n] = *p;
        n++;
    }

    if (out)
        out[n] = '"';
    n++;

    return n;
}

static char *proc_command_line(char *const argv[]) {
    size_t total = 1;

    for (size_t i = 0; argv[i]; i++)
        total += proc_quote_arg(NULL, argv[i]) + 1;

    char *result = malloc(total);

    if (!result)
        return NULL;

    size_t pos = 0;

    for (size_t i = 0; argv[i]; i++) {
        if (i > 0)
            result[pos++] = ' ';

        pos += proc_quote_arg(result + pos, argv[i]);
    }

    result[pos] = '\0';

    return result;
}

static proc_handle_t proc_create(const char *application, char *command_line) {
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;

    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!CreateProcessA(
        application,
        command_line,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &startup_info,
        &process_info
    ))
        return INVALID_PROC_HANDLE;

    CloseHandle(process_info.hThread);

    return process_info.hProcess;
}

proc_handle_t proc_spawn(const char *file, char *const argv[]) {
    (void)file;

    char *command_line = proc_command_line(argv);

    if (!command_line)
        return INVALID_PROC_HANDLE;

    /* NULL application name makes CreateProcess search PATH for argv[0] */
    proc_handle_t handle = proc_create(NULL, command_line);

    if (handle == INVALID_PROC_HANDLE)
        fprintf(stderr, "CreateProcess: error %lu\n", GetLastError());

    free(command_line);

    return handle;
}
int proc_wait(proc_handle_t handle, int *exit_code) {
    if (handle == INVALID_PROC_HANDLE)
        return -1;

    if (WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0)
        return -1;

    DWORD code;

    if (!GetExitCodeProcess(handle, &code))
        return -1;

    if (exit_code != NULL)
        *exit_code = (int)code;

    return 0;
}
int proc_kill(proc_handle_t handle) {
    if (handle == INVALID_PROC_HANDLE)
        return -1;

    return TerminateProcess(handle, 1) ? 0 : -1;
}
void proc_close(proc_handle_t handle) {
    if (handle != INVALID_PROC_HANDLE)
        CloseHandle(handle);
}

#else

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

typedef pid_t proc_handle_t;
#define INVALID_PROC_HANDLE ((pid_t)-1)

proc_handle_t proc_spawn(const char *file, char *const argv[]) {
    pid_t pid = fork();

    if (pid < 0)
        return INVALID_PROC_HANDLE;

    if (pid == 0) {
        execvp(file, argv);

        perror("execvp");
        _exit(127);
    }

    return pid;
}
int proc_wait(proc_handle_t handle, int *exit_code) {
    if (handle == INVALID_PROC_HANDLE)
        return -1;

    int status;

    if (waitpid(handle, &status, 0) < 0)
        return -1;

    if (WIFEXITED(status)) {
        if (exit_code != NULL)
            *exit_code = WEXITSTATUS(status);

        return 0;
    }

    return -1;
}
int proc_kill(proc_handle_t handle) {
    if (handle == INVALID_PROC_HANDLE)
        return -1;

    return kill(handle, SIGKILL);
}
void proc_close(proc_handle_t handle) {
    (void)handle;
}

#endif

#endif
