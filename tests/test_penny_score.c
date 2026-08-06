/* test_penny_score — v2 weights, verified against the Python tool. */
#include <stdio.h>
#include <string.h>

#include "penny_score.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    const char *band = NULL;

    /* Verified: perfect citizen = 1000 First Founders */
    int s = penny_score_compute(1.0, 12, 0, 0, 5, 10, 3, &band);
    CK(s == 1000, "perfect = 1000");
    CK(strcmp(band, PS_BAND_FIRST_FOUNDERS) == 0, "band first founders");

    /* Verified: 764 Crew (the v2 example) */
    s = penny_score_compute(1.0, 12, 1, 0, 3, 5, 2, &band);
    CK(s == 764, "764 crew");
    CK(strcmp(band, PS_BAND_CREW) == 0, "band crew");

    /* Verified: max stays 1000 */
    s = penny_score_compute(1.0, 50, 0, 0, 8, 15, 4, &band);
    CK(s == 1000, "max caps at 1000");

    /* Zero-everything = 0 Lost */
    s = penny_score_compute(0.0, 0, 0, 3, 0, 0, 0, &band);
    CK(s == 0, "zero = 0");
    CK(strcmp(band, PS_BAND_LOST) == 0, "band lost");

    /* v2 weight check: punctuality alone at full = 300 */
    s = penny_score_compute(0.0, 5, 0, 0, 0, 0, 0, NULL);
    CK(s == 300, "punctuality weight = 300");

    printf(fails ? "%d FAILURES\n" : "PENNY_SCORE: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
