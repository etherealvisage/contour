#include <stdio.h>
#include "log/log.h"

#include "parser/parser.h"

#include "prover/interface.h"

int main(int __attribute__((unused)) argc,
        char __attribute__((unused)) *argv[]) {

    contour_log_info("Initializing.");

    struct tree_node *test_parse = parse("(((A>B)&(~B))>(~A))");

    char buffer[1024];
    tree_node_dump(buffer, test_parse);
    printf("Dump:\n%s\n", buffer);

    tree_node_dec(test_parse);

    //test_parse = parse("((B&(~B))>A)");
    test_parse = parse("(((A|(A>_))>_)>_)");
#if 0
    (
        (
            (
                A
                |
                (
                    A
                    >
                    _
                )
            )
            >
            _
        )
        >
        _
    )
#endif
    tree_node_dump(buffer, test_parse);
    printf("Dump:\n%s\n", buffer);

    struct proof_sequent *proof = prove(test_parse);
    if(proof) {
        printf("Provable!\n");
        dump_proof(proof);
    }
    else {
        printf("Unprovable.\n");
    }

    return 0;
}
