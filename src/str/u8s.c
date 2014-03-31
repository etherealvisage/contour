#include <stdlib.h>
#include <string.h>

#include "u8s.h"

U8S_NAME(cptr) U8S_NAME(next)(U8S_NAME(cptr) str) {
    // non-extended character?
    if((*str & 0xc0) == 0x0) return str+1;
    // the annoying case, in the middle of a character
    if((*str & 0xc0) == 0x80) {
        while((*str & 0xc0) == 0x80) str ++;
        return str;
    }
    // beginning of extended character. We know what to do!
    // two-byte?
    if((*str & 0xe0) == 0xc0) return str+2;
    // three-byte?
    if((*str & 0xf0) == 0xe0) return str+3;
    // four-byte?
    if((*str & 0xf8) == 0xf0) return str+4;
    // five-byte?
    if((*str & 0xfc) == 0xf8) return str+5;
    // six-byte?
    if((*str & 0xfe) == 0xfc) return str+6;

    // This should never happen, it's a malformed byte.
    return str+1;
}

size_t U8S_NAME(strlen)(U8S_NAME(cptr) str) {
    size_t result = 0;
    while(*str != 0) {
        str = U8S_NAME(next)(str);
        result ++;
    }
    return result;
}

size_t U8S_NAME(strlenb)(U8S_NAME(cptr) str) {
    size_t result = 0;
    while(*str != 0) str ++, result ++;
    return result;
}

size_t U8S_NAME(strnlenb)(U8S_NAME(cptr) str, size_t bufsiz) {
    size_t result = 0;
    while(*str != 0 && result < bufsiz) str ++, result ++;
    return result;
}

U8S_NAME(ptr) U8S_NAME(strcpy)(U8S_NAME(ptr) target, U8S_NAME(cptr) src) {
    do {
        *(target ++) = *(src ++);
    } while(*src != 0);
    *target = 0;
    return target;
}

U8S_NAME(ptr) U8S_NAME(strncpy)(U8S_NAME(ptr) target, U8S_NAME(cptr) src,
    size_t bufsiz) {

    size_t used = 0;
    while(*src != 0) {
        U8S_NAME(cptr) next = U8S_NAME(next)(src);
        ptrdiff_t len = next-src;
        if(used+len >= bufsiz) break;
        memcpy(target + used, src, len);
        used += len, src += len;
    }
    target[used] = 0;
    return target;
}

int U8S_NAME(strcmp)(U8S_NAME(cptr) a, U8S_NAME(cptr) b) {
    return U8S_NAME(strncmp)(a, b, (unsigned)-1);
}

int U8S_NAME(strncmp)(U8S_NAME(cptr) a, U8S_NAME(cptr) b, size_t bufsiz) {
    return strncmp(a, b, bufsiz);
}

U8S_NAME(ptr) U8S_NAME(strdup)(U8S_NAME(cptr) s) {
    return U8S_NAME(strcpy)(malloc(U8S_NAME(strlenb(s))), s);
}

U8S_NAME(ptr) U8S_NAME(strndup)(U8S_NAME(cptr) s, size_t bufsiz) {
    return U8S_NAME(strncpy)(
        malloc(U8S_NAME(strnlenb)(s, bufsiz)+1),
        s,
        bufsiz+1);
}
