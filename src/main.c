#include <stdio.h>
#include "log/log.h"

#include "parser/parser.h"

int main(int argc, char *argv[]) {
    contour_log_info("Initializing.");

    struct tree_node *test_parse = parse("(((A>B)&(~B))>(~A))");

    char buffer[1024];
    tree_node_dump(buffer, test_parse);
    printf("Dump:\n%s\n", buffer);

    return 0;
}
