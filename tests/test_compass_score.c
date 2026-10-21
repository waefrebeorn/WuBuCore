/* test_compass_score — Compass Score (RND-1), verified against the tanh formula. */
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "compass_score.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 0.5)

int main(void) {
    const char *band = NULL;

    /* Flat (all slopes 0) → 500 Steady */
    int s = compass_score_compute(0.0, 0.0, 0.0, 0.0, &band);
    CK(s == 500, "flat = 500");
    CK(strcmp(band, CMP_BAND_STEADY) == 0, "band steady");

    /* Rising: large positive slopes → near 1000, Rising band */
    s = compass_score_compute(5.0, 3.0, 2.0, 1.0, &band);
    CK(s > 700, "rising slopes = above 700");
    CK(strcmp(band, CMP_BAND_RISING) == 0, "band rising");
    CK(s <= 1000, "caps at 1000");

    /* Falling: large negative slopes → near 0, Falling band */
    s = compass_score_compute(-5.0, -3.0, -2.0, -1.0, &band);
    CK(s < 300, "falling slopes = below 300");
    CK(strcmp(band, CMP_BAND_FALLING) == 0, "band falling");
    CK(s >= 0, "floors at 0");

    /* Single big act can't game: slope of 5.0 in penny alone */
    s = compass_score_compute(5.0, 0.0, 0.0, 0.0, NULL);
    CK(s > 500, "single large slope > 500");
    CK(s < 1000, "single large slope < 1000 (tanh bounded)");

    /* Weight check: penny has 0.4 multiplier in tanh argument */
    /* With slope of 1.0 in penny, argument = 0.4, tanh(0.4) ≈ 0.3799 */
    /* score = 500 + 500 * 0.3799 ≈ 689.95 → 690 */
    s = compass_score_compute(1.0, 0.0, 0.0, 0.0, NULL);
    CK(s == 690, "penny slope 1.0 → ~690");

    /* Verify the formula manually for a known case */
    /* slope = 0.4*0.5 + 0.3*0.5 + 0.2*0.5 + 0.1*0.5 = 0.5 */
    /* tanh(0.5) ≈ 0.4621, score = 500 + 500*0.4621 ≈ 731 */
    double expected = 731.0;
    s = compass_score_compute(0.5, 0.5, 0.5, 0.5, NULL);
    CK(NEAR(s, expected), "equal slopes 0.5 → ~731");

    printf(fails ? "%d FAILURES\n" : "COMPASS_SCORE: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
