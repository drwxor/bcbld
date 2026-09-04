#ifndef CBLD_PROC_H
#define CBLD_PROC_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>

typedef pid_t proc_handle_t;
#define INVALID_PROC_HANDLE ((pid_t)-1)

proc_handle_t proc_spawn(const char *file, char *const argv[])
{
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
int proc_wait(proc_handle_t handle, int *exit_code)
{
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
int proc_kill(proc_handle_t handle)
{
    if (handle == INVALID_PROC_HANDLE)
        return -1;

    return kill(handle, SIGKILL);
}
void proc_close(proc_handle_t handle)
{
    (void)handle;
}

#endif
