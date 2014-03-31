#ifndef CONTOUR_LOG_LOG_H
#define CONTOUR_LOG_LOG_H

enum log_level {
    LOG_INFO,
    LOG_ERROR
};

void contour_log(enum log_level level, const char *string, ...);

#define contour_log_info(...) contour_log(LOG_INFO, __VA_ARGS__)
#define contour_log_error(...) contour_log(LOG_ERROR, __VA_ARGS__)

#endif
