#include "proof.h"
#include "util.h"

struct proof_sequent *sequent_make(int left_count) {
    struct proof_sequent *ret = zalloc(sizeof(*ret));

    ret->left_count = left_count;
    ret->left = zalloc(sizeof(struct tree_node *) * left_count);

    return ret;
}

void sequent_destroy(struct proof_sequent *sequent) {
    if(!sequent) return;

    tree_node_dec(sequent->right);

    for(int i = 0; i < sequent->left_count; i ++) {
        tree_node_dec(sequent->left[i]);
    }

    if(sequent->sleft) sequent_destroy(sequent->sleft);
    if(sequent->sright) sequent_destroy(sequent->sright);
}
