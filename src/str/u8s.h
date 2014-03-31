#ifndef CONTOUR_STR_U8S_H
#define CONTOUR_STR_U8S_H

#include <stddef.h>

#define U8S_NAME(n) str_u8s_ ## n

typedef char * U8S_NAME(ptr);
typedef const char * U8S_NAME(cptr);

/* Get beginning of next UTF-8 character. */
U8S_NAME(cptr) U8S_NAME(next)(U8S_NAME(cptr) str);
/* Get length of UTF-8 string in characters. */
size_t U8S_NAME(strlen)(U8S_NAME(cptr) str);
/* Get length of UTF-8 string in bytes. */
size_t U8S_NAME(strlenb)(U8S_NAME(cptr) str);
/* Get length of UTF-8 string in bytes. */
size_t U8S_NAME(strnlenb)(U8S_NAME(cptr) str, size_t bufsiz);
/* Copy UTF-8 string into target. */
U8S_NAME(ptr) U8S_NAME(strcpy)(U8S_NAME(ptr) target, U8S_NAME(cptr) src);
/* Copy at most bufsiz bytes of the source UTF-8 string into target, respecting
    UTF-8 character boundaries, and ensuring that the target is
    NULL-terminated. */
U8S_NAME(ptr) U8S_NAME(strncpy)(U8S_NAME(ptr) target, U8S_NAME(cptr) src,
    size_t bufsiz);
/* Compare two NULL-terminated UTF-8 strings. */
int U8S_NAME(strcmp)(U8S_NAME(cptr) a, U8S_NAME(cptr) b);
/* Compare at most the first bufsiz bytes of two possibly non-NULL-terminated
    UTF-8 strings. */
int U8S_NAME(strncmp)(U8S_NAME(cptr) a, U8S_NAME(cptr) b, size_t bufsiz);
/* Create a copy of a NULL-terminated UTF-8 string with memory allocated via
    malloc(). */
U8S_NAME(ptr) U8S_NAME(strdup)(U8S_NAME(cptr) s);
/* Create a copy of a NULL-terminated UTF-8 string with memory allocated via
    malloc(). */
U8S_NAME(ptr) U8S_NAME(strndup)(U8S_NAME(cptr) s, size_t bufsiz);

#endif
