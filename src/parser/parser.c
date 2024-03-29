#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "log/log.h"
#include "util.h"

struct lex_state {
    const char *str;
    int pos;
    int eof;
};

static struct tree_node *parse_helper(struct lex_state *state);
static struct tree_node *parse_helper_variable(struct lex_state *state);

static char lex_peek(struct lex_state *state);
static char lex_pop(struct lex_state *state);
static bool lex_match(struct lex_state *state, char c);
static void lex_expect(struct lex_state *state, char c);
static char *lex_name(struct lex_state *state);

struct tree_node *parse(const char *source) {
    // recursive-descent parser
    
    /* grammar:
        exp -> predicate | neg | conj | disj | impl | forall | exists
        variable -> [\w\d]+
        variable_list -> variable | variable,variable_list
        predicate -> [\w\d]+ | [\w\d]+{variable_list}
        neg -> (~exp)
        conj -> (exp&exp)
        disj -> (exp|exp)
        impl -> (exp>exp)
        forall -> (@variable exp)
        exists -> (!variable exp)

        lexing notes:
        - all whitespace ignored.
    */

    struct lex_state lex;
    lex.str = source;
    lex.pos = 0;
    lex.eof = 0;

    return parse_helper(&lex);
}

static struct tree_node *parse_helper(struct lex_state *state) {
    struct tree_node *ret = NULL;
    // one of the various connectives?
    if(lex_match(state, '(')) {
        // negation?
        if(lex_match(state, '~')) {
            // implication to absurdity
            ret = tree_node_make(TREE_NODE_IMPLICATION, 2);

            // recursively parse the middle
            ret->child[0] = parse_helper(state);

            // add absurdity
            ret->child[1] = tree_node_make(TREE_NODE_ABSURDITY, 0);
        }
        // forall?
        else if(lex_match(state, '@')) {
            struct tree_node *var = parse_helper_variable(state);

            ret = tree_node_make(TREE_NODE_FORALL, 2);

            ret->child[0] = var;
            ret->child[1] = parse_helper(state);
        }
        // exists?
        else if(lex_match(state, '!')) {
            struct tree_node *var = parse_helper_variable(state);

            ret = tree_node_make(TREE_NODE_EXISTS, 2);

            ret->child[0] = var;
            ret->child[1] = parse_helper(state);
        }
        // one of the binary connectives, then
        else {
            // first child
            struct tree_node *left = parse_helper(state);
            
            // what's the type?
            if(lex_match(state, '&'))
                ret = tree_node_make(TREE_NODE_CONJUNCTION, 2);
            else if(lex_match(state, '|'))
                ret = tree_node_make(TREE_NODE_DISJUNCTION, 2);
            else if(lex_match(state, '>'))
                ret = tree_node_make(TREE_NODE_IMPLICATION, 2);
            else {
                contour_log_error("Unknown logical connective %c",
                    lex_peek(state));
                exit(1);
            }

            ret->child[0] = left;
            // second child
            ret->child[1] = parse_helper(state);
        }

        // the closing ')'.
        lex_expect(state, ')');
    }
    else if(lex_peek(state) == '_') {
        ret = tree_node_make(TREE_NODE_ABSURDITY, 0);
        lex_match(state, '_');
    }
    // a predicate?
    else if(isalnum(lex_peek(state))) {
        ret = tree_node_make(TREE_NODE_PREDICATE, 0);

        ret->name = lex_name(state);
        // nonzero-place predicate...
        if(lex_match(state, '{')) {
            ret->child_count ++;
            ret->child = realloc(ret->child, ret->child_count);
            ret->child[0] = parse_helper_variable(state);

            // match all the variables
            while(lex_match(state, ',')) {
                ret->child_count ++;
                ret->child = realloc(ret->child, ret->child_count);
                ret->child[ret->child_count-1] = parse_helper_variable(state);
            }
            lex_expect(state, '}');
        }
    }
    else {
        // parse error
        contour_log_error("Don't know what to do with '%c' (%i)",
            lex_peek(state), lex_peek(state));
        exit(1);
    }

    return ret;
}

static struct tree_node *parse_helper_variable(struct lex_state *state) {
    struct tree_node *ret = tree_node_make(TREE_NODE_VARIABLE, 0);

    ret->name = lex_name(state);

    return ret;
}

static char lex_peek(struct lex_state *state) {
    if(state->eof) return 0;
    if(state->str[state->pos] == 0) { state->eof = 1; return 0; }
    if(isspace(state->str[state->pos])) {
        state->pos ++;
        return lex_peek(state);
    }
    return state->str[state->pos];
}

static char lex_pop(struct lex_state *state) {
    char c = lex_peek(state);
    state->pos ++;
    return c;
}

static bool lex_match(struct lex_state *state, char c) {
    if(lex_peek(state) == c) {
        lex_pop(state);
        return true;
    }
    else return false;
}

static void lex_expect(struct lex_state *state, char c) {
    if(!lex_match(state, c)) {
        contour_log_error("Parse error: expected %c, got %c", c,
            lex_peek(state));
        exit(1);
    }
}

static char *lex_name(struct lex_state *state) {
    char buffer[1024];
    size_t length = 0;
    while(isalnum(lex_peek(state))) buffer[length++] = lex_pop(state);
    buffer[length] = 0;

    return strdup(buffer);
}
