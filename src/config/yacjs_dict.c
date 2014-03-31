#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "yacjs_dict.h"

struct YACJS_NAME(dict_entry) {
    char *key;
    void *value;
    uint64_t key_hash;
};

struct YACJS_NAME(dict) {
    struct YACJS_NAME(dict_entry) *entries;
    int entries_size;
    int entries_count;
};

static char *ystrdup(const char *string);
static uint64_t string_hash(const char *string);
static void resize(struct YACJS_NAME(dict) *dict, int newsize);
static void insert_helper(struct YACJS_NAME(dict) *dict, uint64_t hash,
    const char *key, void *value);

struct YACJS_NAME(dict) *YACJS_NAME(dict_make)() {
    struct YACJS_NAME(dict) *result = malloc(sizeof(*result));

    result->entries = NULL;
    result->entries_size = 0;
    result->entries_count = 0;

    return result;
}

void YACJS_NAME(dict_destroy)(struct YACJS_NAME(dict) *dict,
    YACJS_NAME(dict_visitor) visitor) {

    if(visitor) {
        for(int i = 0; i < dict->entries_size; i ++) {
            if(dict->entries[i].key) {
                visitor(dict->entries[i].value);
                free(dict->entries[i].key);
            }
        }
    }
    free(dict->entries);
    free(dict);
}

void YACJS_NAME(dict_set)(struct YACJS_NAME(dict) *dict, const char *key,
    void *value) {

    if(dict->entries_size == dict->entries_count) {
        resize(dict, dict->entries_size*2 + 1);
    }

    uint64_t hash = string_hash(key);
    insert_helper(dict, hash, ystrdup(key), value);
}

void *YACJS_NAME(dict_get)(struct YACJS_NAME(dict) *dict, const char *key) {
    uint64_t hash = string_hash(key);
    for(int i = 0; i < dict->entries_size; i ++) {
        int in = (i+hash) % dict->entries_size;
        if(!dict->entries[in].key) return NULL;
        if(dict->entries[in].key_hash == hash) return dict->entries[in].value;
    }
    return NULL;
}

static char *ystrdup(const char *string) {
    return strcpy(malloc(strlen(string)+1), string);
}

static uint64_t string_hash(const char *string) {
    uint64_t result = 14695981039346656037ULL; 

    while(*string) {
        result ^= *string;
        result *= 1099511628211ULL;
        string ++;
    }

    return result;
}

static void resize(struct YACJS_NAME(dict) *dict, int newsize) {
    struct YACJS_NAME(dict_entry) *oentries = dict->entries;
    int osize = dict->entries_size;
    dict->entries_size = newsize;
    dict->entries_count = 0;
    dict->entries = calloc(newsize, sizeof(oentries[0]));

    for(int i = 0; i < osize; i ++) {
        if(!oentries[i].key) continue;
        insert_helper(dict, oentries[i].key_hash, oentries[i].key,
            oentries[i].value);
    }
}

static void insert_helper(struct YACJS_NAME(dict) *dict, uint64_t hash,
    const char *key, void *value) {
    for(int i = 0; i < dict->entries_size; i ++) {
        int in = (i+hash) % dict->entries_size;
        if(dict->entries[in].key) continue;
        dict->entries[in].key = ystrdup(key);
        dict->entries[in].key_hash = hash;
        dict->entries[in].value = value;
        break;
    }
    dict->entries_count ++;
}
