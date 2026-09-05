#ifndef CBLD_FILE_H
#define CBLD_FILE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *data;
    size_t size;
} file_data_t;

int file_read(const char *path, file_data_t *out) {
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

void file_free(file_data_t *file) {
    if (file == NULL)
        return;

    free(file->data);

    file->data = NULL;
    file->size = 0;
}

#endif
