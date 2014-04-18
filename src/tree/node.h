#ifndef CONTOUR_TREE_NODE_H
#define CONTOUR_TREE_NODE_H

#include <stdio.h>
#include <stdbool.h>

enum tree_node_type {
    TREE_NODE_PREDICATE,
    TREE_NODE_VARIABLE,
    TREE_NODE_CONJUNCTION,
    TREE_NODE_DISJUNCTION,
    TREE_NODE_IMPLICATION,
    TREE_NODE_ABSURDITY,
    TREE_NODE_FORALL,
    TREE_NODE_EXISTS,
    TREE_NODE_TYPES
};

struct tree_node {
    enum tree_node_type type;

    // used in variable/predicate
    char *name;

    struct tree_node **child;
    int child_count;

    int refcount;
};

struct tree_node *tree_node_make(enum tree_node_type type, int child_count);

// Produces ASCII representation of tree node.
// NOTE: this function lacks size information about the buffer, and as such,
// will overflow on large nodes. Should _only_ be used for debugging purposes!
char *tree_node_dump(char *buffer, struct tree_node *node);

// Produces nice compact representation of tree node.
// Returned string should be free()d.
char *tree_node_fmt(struct tree_node *node);

// Produces LaTeX representation of tree node.
// Returned string should be free()d.
char *tree_node_fmt_latex(struct tree_node *node);

/// duplicates an existing tree node; returns ref to caller
struct tree_node *tree_node_dup(struct tree_node *node);

/// refcounting: increment # of references (returns node)
struct tree_node *tree_node_inc(struct tree_node *node);
/// refcountint: decrement # of references
void tree_node_dec(struct tree_node *node);

// returns true if equal
bool tree_node_cmp(struct tree_node *n, struct tree_node *m);

#endif
