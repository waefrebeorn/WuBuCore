/*
 * compass_score.c — the Compass Score (trended direction, 0–1000).
 * Filed RND-1: Compass Score measures where a first one is heading,
 * trended over 24 months.  Parody of FICO 10T / VantageScore 4.0.
 *
 * Formula (RND-1 Part 3):
 *   Compass = 500 + 500 × tanh(
 *       +0.4 × penny_slope
 *       +0.3 × vigilance_slope
 *       +0.2 × creation_slope
 *       +0.1 × crew_slope )
 *
 * Bands (RND-1):
 *   > 700  rising — a first one gathering speed
 *   ~ 500  flat — steady, reliable, fine
 *   < 300  falling — outreach, not penalty
 *
 * The tanh ensures the score is bounded in (0, 1000) and that
 * a single big act cannot fake a slope — the trend is what matters.
 */
#include "compass_score.h"

#include "wubucore.h"

static const char *band_of(int score) {
    if (score > 700) return CMP_BAND_RISING;
    if (score < 300) return CMP_BAND_FALLING;
    return CMP_BAND_STEADY;
}

int compass_score_compute(double penny_slope, double vigilance_slope,
                          double creation_slope, double crew_slope,
                          const char **band_out) {
    double slope = 0.4 * penny_slope +
                   0.3 * vigilance_slope +
                   0.2 * creation_slope +
                   0.1 * crew_slope;

    /* wubu_tanh is the space's tanh (same implementation used
     * in status_loop.c).  Bounded to (-1, 1). */
    double t = wubu_tanh(slope);

    int score = (int)(500.0 + 500.0 * t + 0.5);
    if (score > 1000) score = 1000;
    if (score < 0)   score = 0;

    if (band_out) *band_out = band_of(score);
    return score;
}
