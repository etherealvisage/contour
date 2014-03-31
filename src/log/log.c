#include <stdio.h>
#include <stdarg.h>

#include "log.h"

void contour_log(enum log_level level, const char *string, ...) {
    va_list va;
    va_start(va, string);
    if(level == LOG_INFO) {
        vfprintf(stdout, string, va);
        fprintf(stdout, "\n");
    }
    else {
        vfprintf(stderr, string, va);
        fprintf(stderr, "\n");
    }
    va_end(va);
}
