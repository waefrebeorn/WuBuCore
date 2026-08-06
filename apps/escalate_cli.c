/* escalate_cli — the Fengqiao ladder (GOV-7). */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "escalate.h"

int main(int argc, char **argv) {
    /* usage: escalate COMPLEXITY CAPACITY NAMES_OFFICIAL SPIKE_Z K */
    if (argc == 6) {
        escalate_filing f;
        escalate_result r;
        f.complexity = atof(argv[1]);
        f.capacity = atof(argv[2]);
        f.names_official = atoi(argv[3]);
        f.spike_z = atof(argv[4]);
        f.spike_k = atof(argv[5]);
        if (escalate_decide(&f, &r) != 0) {
            fprintf(stderr, "bad escalate args\n");
            return 2;
        }
        printf("ESCALATION (GOV-7 Fengqiao ladder):\n");
        printf("  complexity %.2f / capacity %.2f%s\n", f.complexity,
               f.capacity, f.names_official ? "  [names official]"
                                            : "");
        printf("  spike z %+.2f (k %.1f)%s\n", f.spike_z, f.spike_k,
               fabs(f.spike_z) > f.spike_k ? "  [SPIKE]" : "");
        printf("  level    : %d (%s)\n", r.level,
               r.level == 2 ? "Bureau" :
               r.level == 1 ? "one level up" : "resolve here");
        printf("  verdict  : %s\n", r.verdict);
        return 0;
    }
    fprintf(stderr, "usage: escalate COMPLEXITY CAPACITY "
                    "NAMES_OFFICIAL SPIKE_Z K\n");
    return 2;
}
