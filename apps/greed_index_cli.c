/* greed_index_cli — compute the Greed Index. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "greed_index.h"

int main(int argc, char **argv) {
    double capture = 0, nexus = 0, malice = 0;
    for (int i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "--capture") == 0) capture = atof(argv[i + 1]);
        else if (strcmp(argv[i], "--nexus") == 0) nexus = atof(argv[i + 1]);
        else if (strcmp(argv[i], "--malice") == 0) malice = atof(argv[i + 1]);
    }
    greed_result r = greed_index_compute(capture, nexus, malice);
    printf("GREED INDEX: %.0f/300\n", r.total);
    printf("  capture : %.0f/100  (unearned value concentration, ECO-4)\n", capture);
    printf("  nexus   : %.0f/100  (the seam, clean inside/dirty outside, ECO-6)\n", nexus);
    printf("  malice  : %.0f/100  (malicious compliance risk)\n", malice);
    printf("  band    : %s\n", r.band);
    printf("  response: %s\n", r.response);
    return 0;
}
