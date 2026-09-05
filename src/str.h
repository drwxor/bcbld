#ifndef CBLD_STR_H
#define CBLD_STR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *strjoin(char *const items[], size_t count, const char *separator) {
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
char *strconcat(const char *a, const char *b) {
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
char **str_unpack(char *data, size_t size, size_t *count) {
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

void str_unpack_free(char **items) {
    free(items);
}

#endif
