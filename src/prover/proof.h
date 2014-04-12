#ifndef CONTOUR_PROVER_PROOF_H
#define CONTOUR_PROVER_PROOF_H

#include "tree/node.h"

struct proof_sequent {
    struct tree_node **left, *right;
    int left_count;

    struct proof_sequent *sleft, *sright;

    const char *tag, *latex_tag;
};

#endif
