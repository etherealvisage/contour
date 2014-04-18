#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log/log.h"

#include "parser/parser.h"

#include "prover/interface.h"

int main(int __attribute__((unused)) argc,
        char __attribute__((unused)) *argv[]) {

    char *buffer = malloc(1048576);
    int bprogress = 0;
    char buf[1024];
    while(fgets(buf, 1024, stdin)) {
        strcpy(buffer + bprogress, buf);
        bprogress += strlen(buf);
    }
    //fgets(buffer, 1048576, stdin);

    struct tree_node *parse_result = parse(buffer);

    struct proof_sequent *proof = prove(parse_result);
    if(proof) {
        latex_proof(proof);
        sequent_destroy(proof);
    }
    else {
        contour_log_info("Unprovable!");
    }

    free(buffer);

    return 0;
}
