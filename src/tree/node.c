#include <string.h>

#include "node.h"
#include "util.h"
#include "log/log.h"

struct strbuild {
    char *s;
    int len;
};

static void strbuild(struct strbuild *str, char c);
static void strbuild_s(struct strbuild *str, const char *s);

static char *tree_node_dump_helper(char *buffer, struct tree_node *node,
    int indent);

static void tree_node_fmt_helper(struct strbuild *str, struct tree_node *node);
static void tree_node_fmt_latex_helper(struct strbuild *str,
    struct tree_node *node);

char *tree_node_dump(char *buffer, struct tree_node *node) {
    return tree_node_dump_helper(buffer, node, 0);
}

static void strbuild(struct strbuild *str, char c) {
    str->s = realloc(str->s, (str->len + 2));
    str->s[str->len] = c;
    str->s[str->len+1] = 0;
    str->len ++;
}

static void strbuild_s(struct strbuild *str, const char *s) {
    while(*s) strbuild(str, *s), s++;
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
        "Absurdity",
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

char *tree_node_fmt(struct tree_node *node) {
    struct strbuild str;
    str.s = NULL;
    str.len = 0;
    tree_node_fmt_helper(&str, node);
    return str.s;
}

static void tree_node_fmt_helper(struct strbuild *str,
    struct tree_node *node) {

    if(node == NULL) {
        strbuild(str, '0');
        return;
    }

    if(node->type == TREE_NODE_PREDICATE) {
        char *s = node->name;
        if(!s) strbuild(str, '=');
        else while(*s) strbuild(str, *s), s++;
    }
    else if(node->type == TREE_NODE_ABSURDITY) {
        strbuild(str, '_');
    }
    else if(node->type == TREE_NODE_IMPLICATION
        || node->type == TREE_NODE_CONJUNCTION
        || node->type == TREE_NODE_DISJUNCTION) {
        
        strbuild(str, '(');
        tree_node_fmt_helper(str, node->child[0]);

        if(node->type == TREE_NODE_IMPLICATION) strbuild(str, '>');
        else if(node->type == TREE_NODE_CONJUNCTION) strbuild(str, '&');
        else if(node->type == TREE_NODE_DISJUNCTION) strbuild(str, '|');

        tree_node_fmt_helper(str, node->child[1]);
        strbuild(str, ')');
    }
    else contour_log_error("???");
}

char *tree_node_fmt_latex(struct tree_node *node) {
    struct strbuild str;
    str.s = NULL;
    str.len = 0;
    tree_node_fmt_latex_helper(&str, node);
    return str.s;
}
static void tree_node_fmt_latex_helper(struct strbuild *str,
    struct tree_node *node) {

    if(node == NULL) {
        strbuild_s(str, "NULL");
        return;
    }

    if(node->type == TREE_NODE_PREDICATE) {
        char *s = node->name;
        if(!s) strbuild_s(str, "(empty name)");
        else strbuild_s(str, s);
    }
    else if(node->type == TREE_NODE_ABSURDITY) {
        strbuild_s(str, "\\bot");
    }
    else if(node->type == TREE_NODE_IMPLICATION
        || node->type == TREE_NODE_CONJUNCTION
        || node->type == TREE_NODE_DISJUNCTION) {
        
        strbuild(str, '(');
        tree_node_fmt_latex_helper(str, node->child[0]);

        if(node->type == TREE_NODE_IMPLICATION) strbuild_s(str, "\\supset ");
        else if(node->type == TREE_NODE_CONJUNCTION) strbuild_s(str, "\\wedge ");
        else if(node->type == TREE_NODE_DISJUNCTION) strbuild_s(str, "\\vee ");

        tree_node_fmt_latex_helper(str, node->child[1]);
        strbuild(str, ')');
    }
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

bool tree_node_cmp(struct tree_node *n, struct tree_node *m) {
    if(n->type != m->type) return false;
    if(n->child_count != m->child_count) return false;
    if(!!n->name != !!m->name) return false;
    if(n->name && strcmp(n->name, m->name)) return false;

    for(int i = 0; i < n->child_count; i ++) {
        if(!tree_node_cmp(n->child[i], m->child[i])) return false;
    }

    return true;
}
