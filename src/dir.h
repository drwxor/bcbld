#ifndef CBLD_DIR_H
#define CBLD_DIR_H

#include <sys/stat.h>
#include <errno.h>

int dir_create(const char *path) {
    if (mkdir(path, 0755) == 0)
        return 0;

    if (errno == EEXIST)
        return 0;

    return -1;
}

#endif
