#ifndef CONTOUR_PROVER_INTERFACE_H
#define CONTOUR_PROVER_INTERFACE_H

#include "proof.h"

struct proof_sequent *prove(struct tree_node *expression);

void dump_proof(struct proof_sequent *sequent);

#endif
