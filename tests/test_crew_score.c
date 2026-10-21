/* test_crew_score — Crew Score (RND-1), verified against documented weights. */
#include <stdio.h>
#include <string.h>

#include "crew_score.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    const char *band = NULL;

    /* Perfect citizen: all at full weight → 1000 */
    int s = crew_score_compute(5, 10, 20, 10, 24, &band);
    CK(s == 1000, "perfect = 1000");
    CK(strcmp(band, CS_BAND_FOUNDERS) == 0, "band first founders");

    /* Zero everything → 0 Drift */
    s = crew_score_compute(0, 0, 0, 0, 0, &band);
    CK(s == 0, "zero = 0");
    CK(strcmp(band, CS_BAND_DRIFT) == 0, "band drift");

    /* Weight verification: coop alone at full = 300 */
    s = crew_score_compute(5, 0, 0, 0, 0, NULL);
    CK(s == 300, "coop weight = 300");

    /* reports alone at full = 250 */
    s = crew_score_compute(0, 10, 0, 0, 0, NULL);
    CK(s == 250, "reports weight = 250");

    /* replies alone at full = 200 */
    s = crew_score_compute(0, 0, 20, 0, 0, NULL);
    CK(s == 200, "replies weight = 200");

    /* mind alone at full = 150 */
    s = crew_score_compute(0, 0, 0, 10, 0, NULL);
    CK(s == 150, "mind weight = 150");

    /* months alone at full = 100 */
    s = crew_score_compute(0, 0, 0, 0, 24, NULL);
    CK(s == 100, "longevity weight = 100");

    /* Max caps at 1000 */
    s = crew_score_compute(50, 50, 50, 50, 120, NULL);
    CK(s == 1000, "max caps at 1000");

    /* Band boundaries */
    s = crew_score_compute(3, 5, 10, 5, 12, &band);  /* should be >= 500 */
    CK(s >= 500, "mid-range >= 500 (First Ones band)");
    CK(strcmp(band, CS_BAND_FIRST) == 0, "band first ones");

    printf(fails ? "%d FAILURES\n" : "CREW_SCORE: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
