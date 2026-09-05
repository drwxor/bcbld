#ifndef CBLD_BUILD_H
#define CBLD_BUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "file.h"
#include "dir.h"
#include "proc.h"
#include "path.h"

static int build(const char *dir_path, const char *cbld_dir) {
    char *build_dir = strconcat(dir_path, "/.build");

    if (!build_dir) {
        return 1;
    }

    if (dir_create(build_dir) < 0) {
        fprintf(stderr, "failed to create .build directory\n");
        free(build_dir);
        return 1;
    }

    char *cbld_sources_path = strconcat(cbld_dir, "/sources");

    char *cbld_cflags_path = strconcat(cbld_dir, "/cflags");
    char *cbld_cxxflags_path = strconcat(cbld_dir, "/cxxflags");
    char *cbld_ldflags_path = strconcat(cbld_dir, "/ldflags");

    char *cbld_target_path = strconcat(cbld_dir, "/target");

    char *cbld_compiler_path = strconcat(cbld_dir, "/compiler");
    char *cbld_linker_path = strconcat(cbld_dir, "/linker");

    char *cbld_includes_path = strconcat(cbld_dir, "/includes");

    if (
        !cbld_sources_path ||
        !cbld_cflags_path ||
        !cbld_cxxflags_path ||
        !cbld_ldflags_path ||
        !cbld_target_path ||
        !cbld_compiler_path ||
        !cbld_linker_path ||
        !cbld_includes_path
    ) {
        free(cbld_sources_path);

        free(cbld_cflags_path);
        free(cbld_cxxflags_path);
        free(cbld_ldflags_path);

        free(cbld_target_path);

        free(cbld_compiler_path);
        free(cbld_linker_path);

        free(cbld_includes_path);

        free(build_dir);

        return 1;
    }

    file_data_t cbld_sources = {0};
    file_data_t cbld_cflags = {0};
    file_data_t cbld_cxxflags = {0};
    file_data_t cbld_ldflags = {0};
    file_data_t cbld_compiler = {0};
    file_data_t cbld_linker = {0};
    file_data_t cbld_target = {0};
    file_data_t cbld_includes = {0};

    char **sources = NULL;
    char **cflags = NULL;
    char **cxxflags = NULL;
    char **includes = NULL;
    char **ldflags = NULL;
    char **compiler = NULL;
    char **linker = NULL;
    char **target = NULL;

    size_t source_count = 0;
    size_t cflags_count = 0;
    size_t cxxflags_count = 0;
    size_t includes_count = 0;
    size_t ldflags_count = 0;
    size_t compiler_count = 0;
    size_t linker_count = 0;
    size_t target_count = 0;

    char **objects = NULL;
    proc_handle_t *processes = NULL;

    char *target_path = NULL;

    if (file_read(cbld_sources_path, &cbld_sources) < 0) {
        fprintf(stderr, "failed to read sources\n");
        goto cleanup;
    }

    if (file_read(cbld_cflags_path, &cbld_cflags) < 0) {
        fprintf(stderr, "failed to read cflags\n");
        goto cleanup;
    }

    if (file_read(cbld_cxxflags_path, &cbld_cxxflags) < 0) {
        fprintf(stderr, "failed to read cxxflags\n");
        goto cleanup;
    }

    if (file_read(cbld_ldflags_path, &cbld_ldflags) < 0) {
        fprintf(stderr, "failed to read ldflags\n");
        goto cleanup;
    }

    if (file_read(cbld_compiler_path, &cbld_compiler) < 0) {
        fprintf(stderr, "failed to read compiler\n");
        goto cleanup;
    }

    if (file_read(cbld_linker_path, &cbld_linker) < 0) {
        fprintf(stderr, "failed to read linker\n");
        goto cleanup;
    }

    if (file_read(cbld_target_path, &cbld_target) < 0) {
        fprintf(stderr, "failed to read target\n");
        goto cleanup;
    }

    if (file_read(cbld_includes_path, &cbld_includes) < 0) {
        fprintf(stderr, "failed to read includes\n");
        goto cleanup;
    }

    sources = str_unpack(
        cbld_sources.data,
        cbld_sources.size,
        &source_count
    );

    if (!sources) {
        fprintf(stderr, "failed to unpack sources\n");
        goto cleanup;
    }

    cflags = str_unpack(
        cbld_cflags.data,
        cbld_cflags.size,
        &cflags_count
    );

    if (!cflags) {
        fprintf(stderr, "failed to unpack cflags\n");
        goto cleanup;
    }

    cxxflags = str_unpack(
        cbld_cxxflags.data,
        cbld_cxxflags.size,
        &cxxflags_count
    );

    if (!cxxflags) {
        fprintf(stderr, "failed to unpack cxxflags\n");
        goto cleanup;
    }

    includes = str_unpack(
        cbld_includes.data,
        cbld_includes.size,
        &includes_count
    );

    if (!includes) {
        fprintf(stderr, "failed to unpack includes\n");
        goto cleanup;
    }

    ldflags = str_unpack(
        cbld_ldflags.data,
        cbld_ldflags.size,
        &ldflags_count
    );

    if (!ldflags) {
        fprintf(stderr, "failed to unpack ldflags\n");
        goto cleanup;
    }

    compiler = str_unpack(
        cbld_compiler.data,
        cbld_compiler.size,
        &compiler_count
    );

    if (!compiler || compiler_count == 0) {
        fprintf(stderr, "failed to read compiler\n");
        goto cleanup;
    }

    linker = str_unpack(
        cbld_linker.data,
        cbld_linker.size,
        &linker_count
    );

    if (!linker || linker_count == 0) {
        fprintf(stderr, "failed to read linker\n");
        goto cleanup;
    }

    target = str_unpack(
        cbld_target.data,
        cbld_target.size,
        &target_count
    );

    if (!target || target_count == 0) {
        fprintf(stderr, "failed to read target\n");
        goto cleanup;
    }

    if (source_count == 0) {
        fprintf(stderr, "no sources specified\n");
        goto cleanup;
    }

    objects = calloc(source_count, sizeof(*objects));
    processes = calloc(source_count, sizeof(*processes));

    if (!objects || !processes) {
        fprintf(stderr, "failed to allocate build data\n");
        goto cleanup;
    }

    char *compiler_str = compiler[0];
    char *linker_str = linker[0];

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

        const char *source = sources[i];
        const char *extension = strrchr(source, '.');

        int is_cpp =
            extension &&
            (
                strcmp(extension, ".cpp") == 0 ||
                strcmp(extension, ".cc") == 0 ||
                strcmp(extension, ".cxx") == 0 ||
                strcmp(extension, ".C") == 0
            );

        size_t flags_count = is_cpp
            ? cxxflags_count
            : cflags_count;

        size_t argc =
            1 +
            includes_count +
            flags_count +
            4;

        char **args = malloc(
            (argc + 1) * sizeof(*args)
        );

        if (!args) {
            fprintf(
                stderr,
                "failed to allocate compiler arguments\n"
            );

            free(source_path);

            goto kill_processes;
        }

        size_t n = 0;
        size_t include_args_count = 0;

        args[n++] = compiler_str;

        for (size_t j = 0; j < includes_count; j++) {
            char *include_path = strconcat(
                dir_path,
                "/"
            );

            if (!include_path) {
                fprintf(
                    stderr,
                    "failed to create include path\n"
                );

                goto compile_cleanup;
            }

            tmp = include_path;

            include_path = strconcat(
                include_path,
                includes[j]
            );

            free(tmp);

            if (!include_path) {
                fprintf(
                    stderr,
                    "failed to create include path\n"
                );

                goto compile_cleanup;
            }

            char *include_arg = strconcat(
                "-I",
                include_path
            );

            free(include_path);

            if (!include_arg) {
                fprintf(
                    stderr,
                    "failed to create include argument\n"
                );

                goto compile_cleanup;
            }

            args[n++] = include_arg;
            include_args_count++;
        }

        if (is_cpp) {
            for (size_t j = 0; j < cxxflags_count; j++)
                args[n++] = cxxflags[j];
        } else {
            for (size_t j = 0; j < cflags_count; j++)
                args[n++] = cflags[j];
        }

        args[n++] = "-c";
        args[n++] = source_path;
        args[n++] = "-o";
        args[n++] = objects[i];

        args[n] = NULL;

        printf("compiling: %s\n", sources[i]);

        processes[i] = proc_spawn(
            compiler_str,
            args
        );

        for (size_t j = 0; j < include_args_count; j++)
            free(args[1 + j]);

        free(args);
        free(source_path);

        if (processes[i] == INVALID_PROC_HANDLE) {
            fprintf(
                stderr,
                "failed to spawn %s for %s\n",
                compiler_str,
                sources[i]
            );

            goto kill_processes;
        }

        spawned++;
        continue;

compile_cleanup:
        for (size_t j = 0; j < include_args_count; j++)
            free(args[1 + j]);

        free(args);
        free(source_path);

        goto kill_processes;
    }

    target_path = strconcat(
        dir_path,
        "/"
    );

    if (!target_path) {
        fprintf(stderr, "failed to create target path\n");
        goto kill_processes;
    }

    char *tmp = target_path;

    target_path = strconcat(
        target_path,
        target[0]
    );

    free(tmp);

    if (!target_path) {
        fprintf(stderr, "failed to create target path\n");
        goto kill_processes;
    }

    int failed = 0;

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
                "%s failed for %s (exit code %d)\n",
                compiler_str,
                sources[i],
                exit_code
            );

            failed = 1;
        }
    }

    if (failed)
        goto cleanup;

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

            goto cleanup;
        }

        size_t n = 0;

        link_args[n++] = linker_str;

        for (size_t i = 0; i < source_count; i++)
            link_args[n++] = objects[i];

        for (size_t i = 0; i < ldflags_count; i++)
            link_args[n++] = ldflags[i];

        link_args[n++] = "-o";
        link_args[n++] = target_path;

        link_args[n] = NULL;

        printf("linking: %s\n", target[0]);

        proc_handle_t link_process = proc_spawn(
            linker_str,
            link_args
        );

        free(link_args);

        if (link_process == INVALID_PROC_HANDLE) {
            fprintf(stderr, "failed to spawn linker\n");
            goto cleanup;
        }

        int link_exit_code;

        if (proc_wait(link_process, &link_exit_code) < 0) {
            fprintf(stderr, "failed to wait for linker\n");
            proc_close(link_process);
            goto cleanup;
        }

        proc_close(link_process);

        if (link_exit_code != 0) {
            fprintf(
                stderr,
                "linking failed (exit code %d)\n",
                link_exit_code
            );

            goto cleanup;
        }
    }

    free(target_path);

    for (size_t i = 0; i < source_count; i++)
        free(objects[i]);

    free(processes);
    free(objects);

    str_unpack_free(target);
    str_unpack_free(ldflags);
    str_unpack_free(cflags);
    str_unpack_free(cxxflags);
    str_unpack_free(sources);
    str_unpack_free(includes);
    str_unpack_free(compiler);
    str_unpack_free(linker);

    file_free(&cbld_target);
    file_free(&cbld_ldflags);
    file_free(&cbld_cflags);
    file_free(&cbld_cxxflags);
    file_free(&cbld_sources);
    file_free(&cbld_includes);
    file_free(&cbld_compiler);
    file_free(&cbld_linker);

    free(cbld_sources_path);

    free(cbld_cflags_path);
    free(cbld_cxxflags_path);
    free(cbld_ldflags_path);

    free(cbld_target_path);

    free(cbld_compiler_path);
    free(cbld_linker_path);

    free(cbld_includes_path);

    free(build_dir);

    return 0;

kill_processes:
    for (size_t i = 0; i < spawned; i++) {
        proc_kill(processes[i]);
        proc_wait(processes[i], NULL);
        proc_close(processes[i]);
    }

cleanup:
    free(target_path);

    if (objects) {
        for (size_t i = 0; i < source_count; i++)
            free(objects[i]);
    }

    free(processes);
    free(objects);

    if (target)
        str_unpack_free(target);

    if (ldflags)
        str_unpack_free(ldflags);

    if (cflags)
        str_unpack_free(cflags);

    if (cxxflags)
        str_unpack_free(cxxflags);

    if (sources)
        str_unpack_free(sources);

    if (includes)
        str_unpack_free(includes);

    if (compiler)
        str_unpack_free(compiler);

    if (linker)
        str_unpack_free(linker);

    file_free(&cbld_target);
    file_free(&cbld_ldflags);
    file_free(&cbld_cflags);
    file_free(&cbld_cxxflags);
    file_free(&cbld_sources);
    file_free(&cbld_includes);
    file_free(&cbld_compiler);
    file_free(&cbld_linker);

    free(cbld_sources_path);

    free(cbld_cflags_path);
    free(cbld_cxxflags_path);
    free(cbld_ldflags_path);

    free(cbld_target_path);

    free(cbld_compiler_path);
    free(cbld_linker_path);

    free(cbld_includes_path);

    free(build_dir);

    return 1;
}

#endif
