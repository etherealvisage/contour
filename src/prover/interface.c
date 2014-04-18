#include <string.h>

#include "interface.h"
#include "rules.h"
#include "util.h"
#include "log/log.h"

static bool prove_dfs(struct proof_sequent *sequent);

static void dump_proof_helper(struct proof_sequent *sequent, int indent);
static void latex_proof_helper(struct proof_sequent *sequent);

static int dfs_count = 0;

struct proof_sequent *prove(struct tree_node *expression) {
    struct proof_sequent *initial_sequent = sequent_make(0);

    initial_sequent->right = expression;

    dfs_count = 0;

    bool result = prove_dfs(initial_sequent);

    contour_log_info("DFS count: %i", dfs_count);

    if(result) {
        return initial_sequent;
    }
    // XXX: leak
    else {
        sequent_destroy(initial_sequent);
        return NULL;
    }
}

static bool prove_dfs(struct proof_sequent *sequent) {
    if(!sequent) return true;

    dfs_count ++;

    int count;
    struct prover_rule_application *results = prover_rules_find(sequent,
        &count);

    if(count == 0) {
        free(results);
        return false;
    }

    bool any_invertible = false;
    for(int i = 0; i < count; i ++) {
        if(results[i].invertible) { any_invertible = true; break; }
    }


    for(int i = 0; i < count; i ++) {
        struct prover_rule_application *app = results + i;

        if(!app->invertible && any_invertible) continue;

        struct prover_rule_result result = app->rule(sequent, app->index);

        // do the right first, it can be smaller.
        if(prove_dfs(result.right) && prove_dfs(result.left)) {
            sequent->sleft = result.left;
            sequent->sright = result.right;
            sequent->tag = app->name;
            sequent->latex_tag = app->latex_name;

            free(results);
            return true;
        }
        else {
            sequent->tag = "impossible";
            sequent->latex_tag = "impossible";

            sequent_destroy(result.left);
            sequent_destroy(result.right);
            if(any_invertible) break;
        }
    }

    free(results);

    return false;
}

void dump_proof(struct proof_sequent *sequent) {
    dump_proof_helper(sequent, 0);
}

static void dump_proof_helper(struct proof_sequent *sequent, int indent) {
    char buffer[1024];
    buffer[0] = 0;
    for(int i = 0; i < sequent->left_count; i ++) {
        char *p = tree_node_fmt(sequent->left[i]);
        if(i != 0) strcat(buffer, ", ");
        strcat(buffer, p);
        free(p);
    }
    
    char *r = tree_node_fmt(sequent->right);

    char prefix[1024];
    for(int i = 0; i < indent; i ++) {
        prefix[i] = '\t';
    }
    prefix[indent] = 0;

    contour_log_info("%s[%s ---> %s] (%s)", prefix, buffer, r,
        sequent->tag?sequent->tag:"");
    free(r);
    
    if(sequent->sleft) dump_proof_helper(sequent->sleft, indent+1);
    if(sequent->sright) dump_proof_helper(sequent->sright, indent+1);
}

void latex_proof(struct proof_sequent *sequent) {
    contour_log_info("\\begin{prooftree}");
    latex_proof_helper(sequent);
    contour_log_info("\\end{prooftree}");
}

static void latex_proof_helper(struct proof_sequent *sequent) {
    if(sequent->sleft) latex_proof_helper(sequent->sleft);
    if(sequent->sright) latex_proof_helper(sequent->sright);

    char buffer[1024];
    buffer[0] = 0;
    for(int i = 0; i < sequent->left_count; i ++) {
        char *p = tree_node_fmt_latex(sequent->left[i]);
        if(i != 0) strcat(buffer, ", ");
        strcat(buffer, p);
        free(p);
    }
    
    char *r = tree_node_fmt_latex(sequent->right);

    const char *type = "        \\UnaryInf";
    if(sequent->sright) type = "        \\BinaryInf";

    if(!sequent->sright && !sequent->sleft) contour_log_info("    \\Axiom$ $");
    contour_log_info("    \\RightLabel{%s}", sequent->latex_tag);
    contour_log_info("%s$%s \\fCenter %s$",
        type, buffer, r);

    free(r);
}
