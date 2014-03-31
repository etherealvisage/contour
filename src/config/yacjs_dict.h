#ifndef YACJS_DICT_H
#define YACJS_DICT_H

#include "yacjs_name.h"

struct YACJS_NAME(dict);

typedef void (*YACJS_NAME(dict_visitor))(void *value);

struct YACJS_NAME(dict) *YACJS_NAME(dict_make)();
void YACJS_NAME(dict_destroy)(struct YACJS_NAME(dict) *dict,
    YACJS_NAME(dict_visitor) visitor);

void YACJS_NAME(dict_set)(struct YACJS_NAME(dict) *dict, const char *key,
    void *value);
void *YACJS_NAME(dict_get)(struct YACJS_NAME(dict) *dict, const char *key);

#endif
