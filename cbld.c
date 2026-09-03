#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>

typedef HANDLE proc_handle_t;
#define INVALID_PROC_HANDLE INVALID_HANDLE_VALUE

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

typedef pid_t proc_handle_t;
#define INVALID_PROC_HANDLE ((pid_t)-1)

#endif

typedef struct {
    char *data;
    size_t size;
} file_data_t;

int file_read(const char *path, file_data_t *out);
void file_free(file_data_t *file);

int dir_create(const char *path);

char *strconcat(const char *a, const char *b);
char *strjoin(char *const items[], size_t count, const char *separator);
char **str_unpack(char *data, size_t size, size_t *count);
void str_unpack_free(char **items);

proc_handle_t proc_spawn(const char *file, char *const argv[]);
int proc_wait(proc_handle_t handle, int *exit_code);
int proc_kill(proc_handle_t handle);
void proc_close(proc_handle_t handle);

char *path_object(const char *build_dir, const char *source);

char *strjoin(char *const items[], size_t count, const char *separator)
{
    size_t total = 1;
    size_t sep_len = strlen(separator);

    for (size_t i = 0; i < count; i++)
        total += strlen(items[i]);

    if (count > 1)
        total += sep_len * (count - 1);

    char *result = malloc(total);
    if (!result)
        return NULL;

    size_t pos = 0;

    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            memcpy(result + pos, separator, sep_len);
            pos += sep_len;
        }

        size_t len = strlen(items[i]);

        memcpy(result + pos, items[i], len);
        pos += len;
    }

    result[pos] = '\0';

    return result;
}

char *strconcat(const char *a, const char *b)
{
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    char *result = malloc(len_a + len_b + 1);

    if (!result)
        return NULL;

    memcpy(result, a, len_a);
    memcpy(result + len_a, b, len_b);

    result[len_a + len_b] = '\0';

    return result;
}

char **str_unpack(char *data, size_t size, size_t *count)
{
    size_t capacity = 8;
    size_t used = 0;
    size_t start = 0;

    char **items = malloc(capacity * sizeof(*items));

    if (!items)
        return NULL;

    for (size_t i = 0; i <= size; i++) {
        if (i != size && data[i] != '\n')
            continue;

        if (i > start && data[i - 1] == '\r')
            data[i - 1] = '\0';
        else if (i < size)
            data[i] = '\0';

        if (data[start] != '\0') {
            if (used + 1 >= capacity) {
                capacity *= 2;

                char **tmp = realloc(
                    items,
                    capacity * sizeof(*items)
                );

                if (!tmp) {
                    free(items);
                    return NULL;
                }

                items = tmp;
            }

            items[used++] = &data[start];
        }

        start = i + 1;
    }

    items[used] = NULL;

    if (count)
        *count = used;

    return items;
}


void str_unpack_free(char **items)
{
    free(items);
}

int file_read(const char *path, file_data_t *out)
{
    FILE *fp;
    long size;
    char *data;

    if (out == NULL)
        return -1;

    out->data = NULL;
    out->size = 0;

    fp = fopen(path, "rb");

    if (fp == NULL)
        return -1;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }

    size = ftell(fp);

    if (size < 0) {
        fclose(fp);
        return -1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    data = malloc((size_t)size + 1);

    if (data == NULL) {
        fclose(fp);
        return -1;
    }

    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return -1;
    }

    data[size] = '\0';

    fclose(fp);

    out->data = data;
    out->size = (size_t)size;

    return 0;
}

void file_free(file_data_t *file)
{
    if (file == NULL)
        return;

    free(file->data);

    file->data = NULL;
    file->size = 0;
}


#ifdef _WIN32

int dir_create(const char *path)
{
    if (CreateDirectoryA(path, NULL))
        return 0;

    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    return -1;
}

static char *build_command_line(char *const argv[])
{
    size_t total_len = 1;

    for (int i = 0; argv[i] != NULL; i++)
        total_len += strlen(argv[i]) + 3;

    char *cmd = malloc(total_len);

    if (cmd == NULL)
        return NULL;

    cmd[0] = '\0';

    for (int i = 0; argv[i] != NULL; i++) {
        if (i > 0)
            strcat(cmd, " ");

        strcat(cmd, "\"");
        strcat(cmd, argv[i]);
        strcat(cmd, "\"");
    }

    return cmd;
}

proc_handle_t proc_spawn(const char *file, char *const argv[])
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    (void)file;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

    char *cmd_line = build_command_line(argv);

    if (cmd_line == NULL)
        return INVALID_PROC_HANDLE;

    BOOL success = CreateProcessA(
        NULL,
        cmd_line,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!success) {
        fprintf(
            stderr,
            "CreateProcessA failed: %lu\n",
            GetLastError()
        );
    }

    free(cmd_line);

    if (!success)
        return INVALID_PROC_HANDLE;

    CloseHandle(pi.hThread);

    return pi.hProcess;
}

int proc_wait(proc_handle_t handle, int *exit_code)
{
    if (handle == INVALID_PROC_HANDLE)
        return -1;

    DWORD result = WaitForSingleObject(handle, INFINITE);

    if (result != WAIT_OBJECT_0)
        return -1;

    DWORD win_exit_code;

    if (!GetExitCodeProcess(handle, &win_exit_code))
        return -1;

    if (exit_code != NULL)
        *exit_code = (int)win_exit_code;

    return 0;
}


int proc_kill(proc_handle_t handle)
{
    if (handle == INVALID_PROC_HANDLE)
        return -1;

    return TerminateProcess(handle, 1) ? 0 : -1;
}


void proc_close(proc_handle_t handle)
{
    if (handle != INVALID_PROC_HANDLE)
        CloseHandle(handle);
}


#else

int dir_create(const char *path)
{
    if (mkdir(path, 0755) == 0)
        return 0;

    if (errno == EEXIST)
        return 0;

    return -1;
}


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

char *path_object(const char *build_dir, const char *source)
{
    const char *name = strrchr(source, '/');

#ifdef _WIN32

    const char *win_name = strrchr(source, '\\');

    if (!name || (win_name && win_name > name))
        name = win_name;

#endif

    if (name)
        name++;
    else
        name = source;

    size_t len = strlen(name);

    if (len >= 2 && strcmp(name + len - 2, ".c") == 0)
        len -= 2;

    char *base = malloc(len + 1);

    if (!base)
        return NULL;

    memcpy(base, name, len);
    base[len] = '\0';

    char *tmp = strconcat(build_dir, "/");

    if (!tmp) {
        free(base);
        return NULL;
    }

    char *result = strconcat(tmp, base);

    free(tmp);
    free(base);

    if (!result)
        return NULL;

    char *object = strconcat(result, ".o");

    free(result);

    return object;
}


int main(int argc, char **argv)
{
    const char *dir_path = argc >= 2 ? argv[1] : ".";

    char *cbld_dir = strconcat(dir_path, "/cbld");

    if (!cbld_dir)
        return 1;

    char *build_dir = strconcat(dir_path, "/.build");

    if (!build_dir) {
        free(cbld_dir);
        return 1;
    }

    if (dir_create(build_dir) < 0) {
        fprintf(stderr, "failed to create .build directory\n");

        free(build_dir);
        free(cbld_dir);

        return 1;
    }

    char *cbld_sources_path = strconcat(cbld_dir, "/sources");
    char *cbld_cflags_path = strconcat(cbld_dir, "/cflags");
    char *cbld_ldflags_path = strconcat(cbld_dir, "/ldflags");
    char *cbld_target_path = strconcat(cbld_dir, "/target");
    char *cbld_includes_path = strconcat(cbld_dir, "/includes");

    if (!cbld_sources_path ||
        !cbld_cflags_path ||
        !cbld_ldflags_path ||
        !cbld_target_path ||
        !cbld_includes_path) {

        free(cbld_sources_path);
        free(cbld_cflags_path);
        free(cbld_ldflags_path);
        free(cbld_target_path);
        free(cbld_includes_path);

        free(build_dir);
        free(cbld_dir);

        return 1;
    }

    file_data_t cbld_sources;

    if (file_read(cbld_sources_path, &cbld_sources) < 0) {
        fprintf(stderr, "failed to read sources\n");
        goto cleanup_paths;
    }

    file_data_t cbld_cflags;

    if (file_read(cbld_cflags_path, &cbld_cflags) < 0) {
        fprintf(stderr, "failed to read cflags\n");
        file_free(&cbld_sources);
        goto cleanup_paths;
    }

    file_data_t cbld_ldflags;

    if (file_read(cbld_ldflags_path, &cbld_ldflags) < 0) {
        fprintf(stderr, "failed to read ldflags");
        file_free(&cbld_cflags);
        file_free(&cbld_sources);
        goto cleanup_paths;
    }

    file_data_t cbld_target;

    if (file_read(cbld_target_path, &cbld_target) < 0) {
        fprintf(stderr, "failed to read target\n");
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_sources);
        goto cleanup_paths;
    }

    file_data_t cbld_includes;

    if (file_read(cbld_includes_path, &cbld_includes) < 0) {
        fprintf(stderr, "failed to read includes\n");
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_includes);
        file_free(&cbld_sources);
        file_free(&cbld_target);
        goto cleanup_paths;
    }

    size_t source_count;

    char **sources = str_unpack(
        cbld_sources.data,
        cbld_sources.size,
        &source_count
    );

    if (!sources) {
        fprintf(stderr, "failed to unpack sources\n");

        file_free(&cbld_target);
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_includes);
        file_free(&cbld_sources);

        goto cleanup_paths;
    }

    size_t cflags_count;

    char **cflags = str_unpack(
        cbld_cflags.data,
        cbld_cflags.size,
        &cflags_count
    );

    if (!cflags) {
        fprintf(stderr, "failed to unpack cflags\n");

        str_unpack_free(sources);

        file_free(&cbld_target);
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_includes);
        file_free(&cbld_sources);

        goto cleanup_paths;
    }

    size_t includes_count;

    char **includes = str_unpack(
        cbld_includes.data,
        cbld_includes.size,
        &includes_count
    );

    if (!includes) {
        fprintf(stderr, "failed to unpack includes\n");

        str_unpack_free(sources);

        file_free(&cbld_target);
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_includes);
        file_free(&cbld_sources);

        goto cleanup_paths;
    }

    size_t ldflags_count;

    char **ldflags = str_unpack(
        cbld_ldflags.data,
        cbld_ldflags.size,
        &ldflags_count
    );

    if (!ldflags) {
        fprintf(stderr, "failed to unpack ldflags\n");

        str_unpack_free(cflags);
        str_unpack_free(sources);

        file_free(&cbld_target);
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_includes);
        file_free(&cbld_sources);

        goto cleanup_paths;
    }

    size_t target_count;

    char **target = str_unpack(
        cbld_target.data,
        cbld_target.size,
        &target_count
    );

    if (!target || target_count == 0) {
        fprintf(stderr, "failed to read target\n");

        str_unpack_free(ldflags);
        str_unpack_free(cflags);
        str_unpack_free(sources);

        file_free(&cbld_target);
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_includes);
        file_free(&cbld_sources);

        goto cleanup_paths;
    }

    if (source_count == 0) {
        fprintf(stderr, "no sources specified\n");

        str_unpack_free(target);
        str_unpack_free(ldflags);
        str_unpack_free(cflags);
        str_unpack_free(sources);

        file_free(&cbld_target);
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_includes);
        file_free(&cbld_sources);

        goto cleanup_paths;
    }

    char **objects = malloc(source_count * sizeof(*objects));
    proc_handle_t *processes =
        malloc(source_count * sizeof(*processes));

    if (!objects || !processes) {
        fprintf(stderr, "failed to allocate build data\n");

        free(objects);
        free(processes);

        str_unpack_free(target);
        str_unpack_free(ldflags);
        str_unpack_free(cflags);
        str_unpack_free(sources);

        file_free(&cbld_target);
        file_free(&cbld_ldflags);
        file_free(&cbld_cflags);
        file_free(&cbld_sources);

        goto cleanup_paths;
    }

    size_t spawned = 0;

    for (size_t i = 0; i < source_count; i++) {
        objects[i] = path_object(
            build_dir,
            sources[i]
        );

        if (!objects[i]) {
            fprintf(
                stderr,
                "failed to create object path for %s\n",
                sources[i]
            );

            goto kill_processes;
        }

        char *source_path = strconcat(
            dir_path,
            "/"
        );

        if (!source_path) {
            fprintf(stderr, "failed to create source path\n");
            goto kill_processes;
        }

        char *tmp = source_path;

        source_path = strconcat(
            source_path,
            sources[i]
        );

        free(tmp);

        if (!source_path) {
            fprintf(stderr, "failed to create source path\n");
            goto kill_processes;
        }

        size_t argc_clang =
            1 +
            cflags_count +
            4;

        char **args = malloc(
            (argc_clang + 1) * sizeof(*args)
        );

        if (!args) {
            fprintf(
                stderr,
                "failed to allocate clang arguments\n"
            );

            free(source_path);

            goto kill_processes;
        }

        size_t n = 0;

        args[n++] = "clang";

        for (size_t j = 0; j < includes_count; j++) {
            char *include_path = strconcat(dir_path, "/");
            if (!include_path)
                goto cleanup_objects;

            char *tmp = include_path;

            include_path = strconcat(include_path, includes[j]);
            free(tmp);

            if (!include_path)
                goto cleanup_objects;

            char *include_arg = strconcat("-I", include_path);
            free(include_path);

            if (!include_arg)
                goto cleanup_objects;

            args[n++] = include_arg;
        }

        for (size_t j = 0; j < cflags_count; j++)
            args[n++] = cflags[j];

        args[n++] = "-c";
        args[n++] = source_path;
        args[n++] = "-o";
        args[n++] = objects[i];

        args[n] = NULL;

        printf("compiling: %s\n", sources[i]);

        processes[i] = proc_spawn(
            "clang",
            args
        );

        free(args);
        free(source_path);

        if (processes[i] == INVALID_PROC_HANDLE) {
            fprintf(
                stderr,
                "failed to spawn clang for %s\n",
                sources[i]
            );

            goto kill_processes;
        }

        spawned++;
    }

    int failed = 0;

    char *target_path = strconcat(dir_path, "/");
    if (!target_path)
        goto cleanup_objects;

    char *tmp = target_path;

    target_path = strconcat(target_path, target[0]);
    free(tmp);

    if (!target_path)
        goto cleanup_objects;

    for (size_t i = 0; i < source_count; i++) {

        int exit_code;

        if (proc_wait(processes[i], &exit_code) < 0) {
            fprintf(
                stderr,
                "failed to wait for %s\n",
                sources[i]
            );

            proc_close(processes[i]);

            failed = 1;

            for (size_t j = i + 1; j < source_count; j++) {
                proc_kill(processes[j]);
                proc_wait(processes[j], NULL);
                proc_close(processes[j]);
            }

            break;
        }

        proc_close(processes[i]);

        if (exit_code != 0) {
            fprintf(
                stderr,
                "clang failed for %s (exit code %d)\n",
                sources[i],
                exit_code
            );

            failed = 1;
        }
    }

    if (failed)
        goto cleanup_objects;

    {
        size_t link_argc =
            1 +
            source_count +
            ldflags_count +
            2;

        char **link_args = malloc(
            (link_argc + 1) * sizeof(*link_args)
        );

        if (!link_args) {
            fprintf(
                stderr,
                "failed to allocate linker arguments\n"
            );

            goto cleanup_objects;
        }

        size_t n = 0;

        link_args[n++] = "clang";

        for (size_t i = 0; i < source_count; i++)
            link_args[n++] = objects[i];

        for (size_t i = 0; i < ldflags_count; i++)
            link_args[n++] = ldflags[i];

        link_args[n++] = "-o";
        link_args[n++] = target_path;

        link_args[n] = NULL;

        printf("linking: %s\n", target[0]);

        proc_handle_t linker = proc_spawn(
            "clang",
            link_args
        );

        free(link_args);

        if (linker == INVALID_PROC_HANDLE) {
            fprintf(stderr, "failed to spawn linker\n");
            goto cleanup_objects;
        }

        int link_exit_code;

        if (proc_wait(linker, &link_exit_code) < 0) {
            fprintf(stderr, "failed to wait for linker\n");
            proc_close(linker);
            goto cleanup_objects;
        }

        proc_close(linker);


        if (link_exit_code != 0) {
            fprintf(
                stderr,
                "linking failed (exit code %d)\n",
                link_exit_code
            );

            goto cleanup_objects;
        }
    }


    for (size_t i = 0; i < source_count; i++)
        free(objects[i]);

    free(processes);
    free(objects);

    str_unpack_free(target);
    str_unpack_free(ldflags);
    str_unpack_free(cflags);
    str_unpack_free(sources);
    str_unpack_free(includes);

    file_free(&cbld_target);
    file_free(&cbld_ldflags);
    file_free(&cbld_cflags);
    file_free(&cbld_sources);
    file_free(&cbld_includes);

    free(cbld_sources_path);
    free(cbld_cflags_path);
    free(cbld_ldflags_path);
    free(cbld_target_path);
    free(cbld_includes_path);
    free(build_dir);
    free(cbld_dir);

    return 0;

kill_processes:
    for (size_t i = 0; i < spawned; i++) {
        proc_kill(processes[i]);
        proc_wait(processes[i], NULL);
        proc_close(processes[i]);
    }

cleanup_objects:
    for (size_t i = 0; i < source_count; i++)
        free(objects[i]);

    free(processes);
    free(objects);

    str_unpack_free(target);
    str_unpack_free(ldflags);
    str_unpack_free(cflags);
    str_unpack_free(sources);
    str_unpack_free(includes);

    file_free(&cbld_target);
    file_free(&cbld_ldflags);
    file_free(&cbld_cflags);
    file_free(&cbld_sources);
    file_free(&cbld_includes);

cleanup_paths:
    free(cbld_sources_path);
    free(cbld_cflags_path);
    free(cbld_ldflags_path);
    free(cbld_target_path);
    free(cbld_includes_path);

    free(build_dir);
    free(cbld_dir);

    return 1;
}
