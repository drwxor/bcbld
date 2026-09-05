#ifndef CBLD_PATH_H
#define CBLD_PATH_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"

char *path_object(const char *build_dir, const char *source) {
    const char *name = strrchr(source, '/');


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


#endif
