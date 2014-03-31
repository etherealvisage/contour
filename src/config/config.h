#ifndef CONTOUR_CONFIG_CONFIG_H
#define CONTOUR_CONFIG_CONFIG_H

void config_init(const char *path);
void config_destroy();

const char *config_str(const char *path);
int config_int(const char *path);

#endif
