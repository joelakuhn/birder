#include <string.h>
#include <stdlib.h>

#include "str_vec.h"

str_vec_t* str_vec_new() {
    str_vec_t* str_vec = malloc(sizeof(str_vec_t));
    str_vec->strs = malloc(sizeof(char*) * 10);
    str_vec->allocated = 10;
    str_vec->len = 0;
    return str_vec;
}

void str_vec_push(str_vec_t* str_vec, char* str) {
    if ((str_vec->len + 1) >= str_vec->allocated) {
        str_vec->strs = (char**) realloc(str_vec->strs, (str_vec->allocated + 10) * sizeof(char*));
        str_vec->allocated += 10;
    }
    str_vec->strs[str_vec->len] = str;
    str_vec->len++;
}

char* str_vec_pop(str_vec_t* str_vec) {
    if (str_vec->len > 0) {
        char* str = str_vec->strs[str_vec->len - 1];
        str_vec->strs[str_vec->len - 1] = 0;
        str_vec->len--;
        return str;
    }
    else {
        return (char*) 0;
    }
}

char* str_vec_shift(str_vec_t* str_vec) {
    if (str_vec->len > 0) {
        char* str = str_vec->strs[0];
        char** new_strs = malloc(sizeof(char*) * str_vec->allocated);

        memcpy(new_strs, str_vec->strs + 1, (str_vec->allocated - 1) * sizeof(char*));
        free(str_vec->strs);
        str_vec->strs = new_strs;

        str_vec->len--;
        return str;
    }
    else {
        return 0;
    }
}

str_vec_t* str_vec_dup(str_vec_t* str_vec) {
    str_vec_t* new_str_vec = str_vec_new();
    for (size_t i = 0; i < str_vec->len; i++) {
        str_vec_push(new_str_vec, str_vec->strs[i]);
    }
    return new_str_vec;
}

void str_vec_destroy(str_vec_t* str_vec) {
    free(str_vec->strs);
    free(str_vec);
}
