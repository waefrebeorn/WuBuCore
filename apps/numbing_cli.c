/* numbing_cli — run one numbing world. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "numbing.h"

int main(int argc, char **argv) {
    const char *mode = (argc > 1) ? argv[1] : "nexus";
    int gens = (argc > 3 && strcmp(argv[2], "--gens") == 0) ? atoi(argv[3]) : 5;

    numbing_world world;
    if (strcmp(mode, "calm") == 0) world = NUMBING_CALM;
    else if (strcmp(mode, "modern") == 0) world = NUMBING_MODERN;
    else world = NUMBING_NEXUS;

    numbing_result r;
    if (numbing_run(world, gens, &r) != 0) return 1;
    printf("=== %s ===\n", r.label);
    printf("after %d generations:\n", gens);
    printf("  reward sensitivity: %.2f %s\n", r.final_sensitivity,
           r.numbed ? "(STRUCTURALLY NUMBED - downregulated; even protection "
                       "cannot fully reverse it)"
                    : "(still sensitive)");
    return 0;
}
