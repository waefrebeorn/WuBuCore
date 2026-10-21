/* compass_score_cli — Compass Score compute (0–1000, trended). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compass_score.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "usage: compass_score --penny_slope F --vigilance_slope F "
                "--creation_slope F --crew_slope F\n");
        return 2;
    }
    double penny = 0.0, vigilance = 0.0, creation = 0.0, crew = 0.0;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc) break;
        if (strcmp(argv[i], "--penny_slope") == 0)
            penny = atof(argv[i + 1]);
        else if (strcmp(argv[i], "--vigilance_slope") == 0)
            vigilance = atof(argv[i + 1]);
        else if (strcmp(argv[i], "--creation_slope") == 0)
            creation = atof(argv[i + 1]);
        else if (strcmp(argv[i], "--crew_slope") == 0)
            crew = atof(argv[i + 1]);
    }
    const char *band = NULL;
    int score = compass_score_compute(penny, vigilance, creation, crew, &band);
    printf("Compass Score: %d/1000\n", score);
    printf("Band:          %s\n", band ? band : "");
    return 0;
}
