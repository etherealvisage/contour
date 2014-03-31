#include <string.h>

#include "node.h"
#include "util.h"

static char *tree_node_dump_helper(char *buffer, struct tree_node *node,
    int indent);

char *tree_node_dump(char *buffer, struct tree_node *node) {
    return tree_node_dump_helper(buffer, node, 0);
}

static char *tree_node_dump_helper(char *buffer, struct tree_node *node,
    int indent) {

    for(int i = 0; i < indent; i ++) buffer[0] = '\t', buffer ++;

    const char *headers[] = {
        "Predicate ",
        "Variable ",
        "Conjunction",
        "Disjunction",
        "Implication",
        "Negation",
        "Forall ",
        "Exists ",
        "TYPES"
    };

    strcpy(buffer, headers[node->type]);
    buffer += strlen(headers[node->type]);
    if(node->name) {
        strcpy(buffer, node->name);
        buffer += strlen(node->name);
    }

    buffer[0] = '\n';
    buffer ++;

    for(int i = 0; i < node->child_count; i ++) {
        buffer = tree_node_dump_helper(buffer, node->child[i], indent+1);
    }

    buffer[0] = 0;

    return buffer;
}

struct tree_node *tree_node_dup(struct tree_node *node) {
    struct tree_node *ret = zalloc(sizeof(*ret));

    ret->type = node->type;

    ret->child_count = node->child_count;
    ret->child = memdup(node->child,
        sizeof(struct tree_node *) * node->child_count);
    for(int i = 0; i < ret->child_count; i ++) {
        ret->child[i] = tree_node_dup(ret->child[i]);
    }

    ret->refcount = 1;

    if(node->name) ret->name = strdup(node->name);

    return ret;
}

void tree_node_inc(struct tree_node *node) {
    node->refcount ++;
}

/// refcountint: decrement # of references
void tree_node_dec(struct tree_node *node) {
    node->refcount --;
    if(node->refcount != 0) return;

    for(int i = 0; i < node->child_count; i ++) {
        tree_node_dec(node->child[i]);
    }

    if(node->name) free(node->name);

    free(node->child);
    node->child = NULL; // in case of dangling refs
    free(node);
}
