#include <stdlib.h>
#include <string.h> // memcpy

#include "util.h"

void *memdup(const void *source, size_t len) {
    void *ret = malloc(len);
    memcpy(ret, source, len);
    return ret;
}
