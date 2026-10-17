#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "build.h"
#include "custom.h"

enum Command {
    COMMAND_UNKNOWN,
    COMMAND_BUILD,
    COMMAND_HELP,
    COMMAND_CUSTOM
};

enum Command parse_command(int argc, char **argv) {
    if (argc < 2)
        return COMMAND_UNKNOWN;

    if (strcmp(argv[1], "build") == 0)
        return COMMAND_BUILD;

    if (strcmp(argv[1], "cmd") == 0)
        return COMMAND_CUSTOM;

    if (strcmp(argv[1], "help") == 0)
        return COMMAND_HELP;

    return COMMAND_UNKNOWN;
}

int main(int argc, char **argv) {
    const enum Command command = parse_command(argc, argv);

    switch (command) {
        case COMMAND_BUILD: {
            const char *dir_path = argc >= 3 ? argv[2] : ".";

            char *bcbld_dir = strconcat(dir_path, "/bcbld");
            if (!bcbld_dir)
                return 1;

            int result = build(dir_path, bcbld_dir);

            free(bcbld_dir);
            return result;
        }

        case COMMAND_CUSTOM: {
            const char *dir_path = argc >= 4 ? argv[3] : ".";

            return custom(argc, argv, dir_path);
        }

        case COMMAND_HELP: {
            printf("%s\n", argv[0]);
            printf("\tbuild [directory] - build project\n");
            printf("\tcmd <command> [directory] - run custom command\n");
            printf("\thelp - print this\n");

            return 0;
        }

        case COMMAND_UNKNOWN:
            printf("bcbld: unknown command\n");
            return 1;
    }

    return 1;
}
