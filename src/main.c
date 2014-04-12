#include <stdio.h>
#include <stdlib.h>
#include "log/log.h"

#include "parser/parser.h"

#include "prover/interface.h"

int main(int __attribute__((unused)) argc,
        char __attribute__((unused)) *argv[]) {

    char *buffer = malloc(1048576);
    fgets(buffer, 1048576, stdin);

    struct tree_node *parse_result = parse(buffer);

    struct proof_sequent *proof = prove(parse_result);
    if(proof) {
        latex_proof(proof);
    }
    else {
        printf("Unprovable!\n");
    }

    return 0;
}
