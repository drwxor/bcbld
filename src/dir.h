#ifndef CBLD_DIR_H
#define CBLD_DIR_H

#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#endif

int dir_create(const char *path) {
#ifdef _WIN32
    if (_mkdir(path) == 0)
        return 0;
#else
    if (mkdir(path, 0755) == 0)
        return 0;
#endif

    if (errno == EEXIST)
        return 0;

    return -1;
}

#endif
