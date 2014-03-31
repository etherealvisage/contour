#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "log/log.h"
#include "yacjs.h"

struct config_yacjs_node *config_root;

void config_init(const char *path) {
    contour_log(LOG_INFO, "Loading configuration from %s\n", path);

    char *contents;
    FILE *cf = fopen(path, "r");
    if(cf == NULL) {
        contour_log(LOG_ERROR, "Couldn't open configuration file '%s': %m\n",
            path);
        exit(1);
    }

    fseek(cf, 0, SEEK_END);
    int size = ftell(cf);
    fseek(cf, 0, SEEK_SET);

    contents = malloc(size+1);
    if(fread(contents, 1, size, cf) != (size_t)size) {
        contour_log(LOG_ERROR, "Couldn't read configuration file: %m\n");
        exit(1);
    }
    contents[size] = 0;

    config_yacjs_parse(contents);

    free(contents);
}

void config_destroy() {
    config_yacjs_destroy(config_root);
}

const char *config_str(const char *path) {
    // duplicate the string
    char *npath = strdup(path);
    // walk the tree
    char *save;
    struct config_yacjs_node *cursor = config_root;

    char *p = strtok_r(npath, ".", &save);
    do {
        cursor = config_yacjs_node_dict_get(cursor, p);
        if(cursor == NULL) {
            contour_log_error("Could not find config string \"%s\"", path);
            free(npath);
            return NULL;
        }
    } while((p = strtok_r(NULL, ".", &save)));

    free(npath);

    return config_yacjs_node_str(cursor);
}

int config_int(const char *path) {
    const char *str = config_str(path);
    if(str == NULL) return -1;

    return strtol(str, NULL, 0);
}
