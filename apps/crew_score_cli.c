/* crew_score_cli — Crew Score compute (0–1000). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crew_score.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "usage: crew_score --coop N --reports N --replies N "
                "--mind N --months N\n");
        return 2;
    }
    int coop = 0, reports = 0, replies = 0, mind = 0, months = 0;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) break;
        if (strcmp(argv[i], "--coop") == 0)       coop = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--reports") == 0) reports = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--replies") == 0) replies = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--mind") == 0)    mind = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--months") == 0)  months = atoi(argv[i + 1]);
    }
    const char *band = NULL;
    int score = crew_score_compute(coop, reports, replies, mind, months, &band);
    printf("Crew Score: %d/1000\n", score);
    printf("Band:       %s\n", band ? band : "");
    return 0;
}
