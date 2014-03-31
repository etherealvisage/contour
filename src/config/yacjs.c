#include <stdbool.h>
#include <stdio.h> // for debugging
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "yacjs.h"
#include "yacjs_dict.h"
#include "str/u8s.h"

struct YACJS_NAME(node) {
    enum YACJS_NAME(node_type) type;
    union {
        const char *string;
        int64_t number;
        double fp;
        struct {
            struct YACJS_NAME(node) *entries;
            int entries_count;
            int entries_size;
        } array;
        struct YACJS_NAME(dict) *dict;
        bool boolean;
    } data;
};

enum token_type {
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_FLOAT,
    TOKEN_FPNUMBER,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_OPENDICT,
    TOKEN_CLOSEDICT,
    TOKEN_OPENARRAY,
    TOKEN_CLOSEARRAY,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_NONE,
    TOKEN_ERROR,
    TOKEN_TYPES
};

static enum YACJS_NAME(error) last_error = YACJS_NAME_CAP(ERROR_NONE);

static void destroy_helper(struct YACJS_NAME(node) *node);
static void skip_whitespace(const char ** const ptr);
static const char *next_token(const char ** const ptr, int *length,
    enum token_type *type);
static const char *peek_token(const char ** const ptr, int *length,
    enum token_type *type);
static struct YACJS_NAME(node) *parse_any(const char **string);
static struct YACJS_NAME(node) *parse_dict_contents(const char **string);
static struct YACJS_NAME(node) *parse_array_contents(const char **string);

enum YACJS_NAME(error) YACJS_NAME(last_error)() {
    enum YACJS_NAME(error) err = last_error;
    last_error = YACJS_NAME_CAP(ERROR_NONE);
    return err;
}

struct YACJS_NAME(node) *YACJS_NAME(parse)(const char *string) {
    struct YACJS_NAME(node) *ret = parse_any(&string);
    // ensure there's nothing afterwards.
    skip_whitespace(&string);
    if(string[0] != 0) {
        YACJS_NAME(destroy)(ret);
        last_error = YACJS_NAME_CAP(ERROR_PARSE);
        return NULL;
    }
    return ret;
}

void YACJS_NAME(destroy)(struct YACJS_NAME(node) *node) {
    if(!node) return;
    destroy_helper(node);
    free(node);
}

static void destroy_helper(struct YACJS_NAME(node) *node) {
    if(node->type == YACJS_NAME_CAP(NODE_ARRAY)) {
        for(int i = 0; i < node->data.array.entries_count; i ++) {
            YACJS_NAME(destroy)(node->data.array.entries + i);
        }
        free(node->data.array.entries);
    }
    else if(node->type == YACJS_NAME_CAP(NODE_DICT)) {
        YACJS_NAME(dict_destroy)(node->data.dict,
            (YACJS_NAME(dict_visitor))destroy_helper);
    }
}

enum YACJS_NAME(node_type) YACJS_NAME(node_type)(
    struct YACJS_NAME(node) *node) {

    return node->type;
}

bool YACJS_NAME(node_bool)(struct YACJS_NAME(node) *node) {
    if(node->type != YACJS_NAME_CAP(NODE_BOOLEAN)) {
        last_error = YACJS_NAME_CAP(ERROR_TYPE);
        return NULL;
    }
    return node->data.boolean;
}

const char *YACJS_NAME(node_str)(struct YACJS_NAME(node) *node) {
    if(node->type != YACJS_NAME_CAP(NODE_STRING)) {
        last_error = YACJS_NAME_CAP(ERROR_TYPE);
        return NULL;
    }
    return node->data.string;
}

int64_t YACJS_NAME(node_num)(struct YACJS_NAME(node) *node) {
    if(node->type != YACJS_NAME_CAP(NODE_NUMBER)) {
        last_error = YACJS_NAME_CAP(ERROR_TYPE);
        return -1;
    }
    return node->data.number;
}

double YACJS_NAME(node_float)(struct YACJS_NAME(node) *node) {
    if(node->type != YACJS_NAME_CAP(NODE_FLOAT)) {
        last_error = YACJS_NAME_CAP(ERROR_TYPE);
        return 1/0.0; // NaN
    }
    return node->data.fp;
}

int YACJS_NAME(node_array_size)(struct YACJS_NAME(node) *node) {
    if(node->type != YACJS_NAME_CAP(NODE_ARRAY)) {
        last_error = YACJS_NAME_CAP(ERROR_TYPE);
        return -1;
    }
    return node->data.array.entries_count;
}

struct YACJS_NAME(node) *YACJS_NAME(node_array_elem)(
    struct YACJS_NAME(node) *node, int index) {

    if(node->type != YACJS_NAME_CAP(NODE_ARRAY)) {
        last_error = YACJS_NAME_CAP(ERROR_TYPE);
        return NULL;
    }
    if(index >= node->data.array.entries_count || index < 0) {
        last_error = YACJS_NAME_CAP(ERROR_BOUNDS);
        return NULL;
        
    }
    return node->data.array.entries + index;
}

struct YACJS_NAME(node) *YACJS_NAME(node_dict_get)(
    struct YACJS_NAME(node) *node, const char *key) {

    if(node->type != YACJS_NAME_CAP(NODE_DICT)) {
        last_error = YACJS_NAME_CAP(ERROR_TYPE);
        return NULL;
    }

    return YACJS_NAME(dict_get)(node->data.dict, key);
}

static void skip_whitespace(const char ** const ptr) {
    while((**ptr == ' ' || **ptr == '\t' || **ptr == '\n') && **ptr != 0) {
        *ptr = U8S_NAME(next)(*ptr);
    }
}

static const char *next_token(const char ** const ptr, int *length,
    enum token_type *type) {

    *length = 0;
    // skip whitespace
    skip_whitespace(ptr);
    if(*ptr == 0) {
        *type = TOKEN_NONE;
        return NULL;
    }

    if(**ptr == '"') {
        // TODO: parse string PROPERLY
        const char *start = ++(*ptr);
        *length = 0;
        while(**ptr != '"' && **ptr != 0) {
            (*ptr) ++, (*length) ++;
        }
        if(**ptr == 0) return NULL;
        // skip closing quotes
        else (*ptr)++;

        *type = TOKEN_STRING;
        return start;
    }
    else if(isdigit(**ptr) || **ptr == '-') {
        const char *start = (*ptr);
        *length = 0;
        bool is_fp = false;
        while((isdigit(**ptr) || **ptr == '.' || **ptr == 'e' || **ptr == '-')
            && **ptr != 0) {

            if(**ptr == '.' || **ptr == 'e') is_fp = true;
            (*ptr) ++, (*length) ++;
        }
        if(*ptr == 0) return NULL;
        if(is_fp) *type = TOKEN_FLOAT;
        else *type = TOKEN_NUMBER;

        return start;
    }
    else if(**ptr == '{') {
        *type = TOKEN_OPENDICT;
        return (*ptr)++;
    }
    else if(**ptr == '}') {
        *type = TOKEN_CLOSEDICT;
        return (*ptr)++;
    }
    else if(**ptr == '[') {
        *type = TOKEN_OPENARRAY;
        return (*ptr)++;
    }
    else if(**ptr == ']') {
        *type = TOKEN_CLOSEARRAY;
        return (*ptr)++;
    }
    else if(**ptr == ',') {
        *type = TOKEN_COMMA;
        return (*ptr)++;
    }
    else if(**ptr == ':') {
        *type = TOKEN_COLON;
        return (*ptr)++;
    }
    else if(!strncmp(*ptr, "false", 5)) {
        *type = TOKEN_FALSE;
        return (*ptr) += 5;
    }
    else if(!strncmp(*ptr, "true", 4)) {
        *type = TOKEN_TRUE;
        return (*ptr) += 4;
    }
    else if(!strncmp(*ptr, "null", 4)) {
        *type = TOKEN_NULL;
        return (*ptr) += 4;
    }

    printf("Don't know what to do with a '%c'\n", **ptr);
    printf("context: \n\n<\n%s\n>\n\n", *ptr);

    *type = TOKEN_ERROR;

    return NULL;
}

static const char *peek_token(const char ** const ptr, int *length,
    enum token_type *type) {

    const char *s = *ptr;

    return next_token(&s, length, type);
}

static struct YACJS_NAME(node) *parse_any(const char **string) {
    enum token_type type;
    const char *s;
    int len;

    s = next_token(string, &len, &type);
    if(type == TOKEN_OPENDICT) return parse_dict_contents(string);
    else if(type == TOKEN_OPENARRAY) return parse_array_contents(string);
    else if(type == TOKEN_STRING) {
        struct YACJS_NAME(node) *build = malloc(sizeof(*build));
        build->type = YACJS_NAME_CAP(NODE_STRING);
        build->data.string = U8S_NAME(strndup)(s, len);
        return build;
    }
    else if(type == TOKEN_NUMBER) {
        struct YACJS_NAME(node) *build = malloc(sizeof(*build));
        build->type = YACJS_NAME_CAP(NODE_NUMBER);
        build->data.number = strtoll(s, NULL, 0);
        return build;
    }
    else if(type == TOKEN_FLOAT) {
        struct YACJS_NAME(node) *build = malloc(sizeof(*build));
        build->type = YACJS_NAME_CAP(NODE_FLOAT);
        char *e;
        build->data.fp = strtod(s, &e);
        if(e != *string) {
            free(build);
            // error parsing number, not everything was used
            return NULL;
        }
        return build;
    }
    else if(type == TOKEN_FALSE || type == TOKEN_TRUE) {
        struct YACJS_NAME(node) *build = malloc(sizeof(*build));
        build->type = YACJS_NAME_CAP(NODE_BOOLEAN);
        build->data.boolean = type == TOKEN_TRUE;
        return build;
    }
    else if(type == TOKEN_NULL) {
        struct YACJS_NAME(node) *build = malloc(sizeof(*build));
        build->type = YACJS_NAME_CAP(NODE_NULL);
        return build;
    }

    printf("Unknown token type %i\n", type);

    return NULL;
}

static struct YACJS_NAME(node) *parse_dict_contents(const char **string) {
    enum token_type type;
    const char *s;
    int len;

    struct YACJS_NAME(node) *result = malloc(sizeof(*result));
    result->type = YACJS_NAME_CAP(NODE_DICT);
    result->data.dict = YACJS_NAME(dict_make)();

    while((s = next_token(string, &len, &type))) {
        // closing dictionary token
        if(type == TOKEN_CLOSEDICT) break;
        // we expect a string here if it's not a closing dictionary
        else if(type != TOKEN_STRING) {
            return NULL;
        }

        char *ds = U8S_NAME(strndup)(s, len);

        // expect a colon after the name
        next_token(string, &len, &type);
        if(type != TOKEN_COLON) {
            return NULL;
        }

        struct YACJS_NAME(node) *value = parse_any(string);

        if(value == NULL) {
            // TODO: leaked memory
            return NULL;
        }

        YACJS_NAME(dict_set)(result->data.dict, ds, value);
        free(ds);

        peek_token(string, &len, &type);
        if(type == TOKEN_COMMA) {
            // NOTE: this means things like {"a":1,} are accepted
            // eat the comma
            next_token(string, &len, &type);
        }
    }
    if(!s) {
        // TODO: leaked memory
        return NULL;
    }

    return result;
}

static struct YACJS_NAME(node) *parse_array_contents(const char **string) {
    enum token_type type;
    const char *s;
    int len;

    struct YACJS_NAME(node) *result = malloc(sizeof(*result));
    result->type = YACJS_NAME_CAP(NODE_ARRAY);
    result->data.array.entries = NULL;
    result->data.array.entries_size = 0;
    result->data.array.entries_count = 0;

    while((s = peek_token(string, &len, &type))) {
        // closing dictionary token
        if(type == TOKEN_CLOSEARRAY) {
            // eat first
            next_token(string, &len, &type);
            break;
        }
        struct YACJS_NAME(node) *next = parse_any(string);
        if(!next) {
            // TODO: leaked memory
            return NULL;
        }

        if(result->data.array.entries_size
            == result->data.array.entries_count) {

            void *nmem = realloc(result->data.array.entries,
                sizeof(struct YACJS_NAME(node))
                    * (result->data.array.entries_size * 2 + 1));
            if(nmem == NULL) {
                last_error = YACJS_NAME_CAP(ERROR_MEMORY);
                return NULL;
            }

            result->data.array.entries_size *= 2;
            result->data.array.entries_size ++;
            result->data.array.entries = nmem;
        }
        result->data.array.entries[result->data.array.entries_count++] = *next;
        free(next);

        peek_token(string, &len, &type);
        if(type == TOKEN_COMMA) {
            // NOTE: this means things like [1,2,] are accepted
            // eat the comma
            next_token(string, &len, &type);
        }
    }
    if(!s) {
        // TODO: leaked memory
        return NULL;
    }

    return result;
}
