/* penny_score_cli — Penny Score compute. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "penny_score.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "usage: penny_score --literacy F --punctual N --late N "
                "--missed N --vigilance N --creation N --years N\n");
        return 2;
    }
    double literacy = 0.0;
    int punctual = 0, late = 0, missed = 0, vig = 0, cre = 0, yrs = 0;
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "--literacy") == 0) literacy = atof(argv[i + 1]);
        else if (strcmp(argv[i], "--punctual") == 0) punctual = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--late") == 0) late = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--missed") == 0) missed = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--vigilance") == 0) vig = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--creation") == 0) cre = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--years") == 0) yrs = atoi(argv[i + 1]);
    }
    const char *band = NULL;
    int score = penny_score_compute(literacy, punctual, late, missed,
                                    vig, cre, yrs, &band);
    printf("Penny Score: %d/1000\n", score);
    printf("Band:        %s\n", band ? band : "");
    return 0;
}
