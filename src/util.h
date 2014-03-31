#ifndef CONTOUR_UTIL_H
#define CONTOUR_UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define zalloc(size) calloc(1, size)

void *memdup(const void *source, size_t len);

#endif
