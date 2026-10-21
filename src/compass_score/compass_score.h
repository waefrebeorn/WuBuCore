/*
 * compass_score.h — the Compass Score (0–1000), trended direction.
 * Filed RND-1: Compass Score measures where a first one is heading,
 * trended over 24 months.  Parody of FICO 10T / VantageScore 4.0
 * trended models.
 */
#ifndef COMPASS_SCORE_H
#define COMPASS_SCORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Band labels (caller must not free). */
#define CMP_BAND_RISING  "Rising - a first one gathering speed (above 700)"
#define CMP_BAND_STEADY  "Steady - flat, reliable, fine (500 ± ~200)"
#define CMP_BAND_FALLING "Falling - drifting; outreach, not penalty (below 300)"

/*
 * Compute the Compass Score.
 *  penny_slope      quarterly slope of penny output (per quarter).
 *  vigilance_slope  quarterly slope of verified reports.
 *  creation_slope   quarterly slope of merged works.
 *  crew_slope       quarterly slope of cooperative acts.
 *
 * Formula (RND-1 Part 3):
 *   Compass = 500 + 500 × tanh(
 *       +0.4 × penny_slope
 *       +0.3 × vigilance_slope
 *       +0.2 × creation_slope
 *       +0.1 × crew_slope )
 *
 * Each slope is in units of "change per quarter".  A slope of 1.0
 * means the citizen's metric rose by its full-weight equivalent
 * each quarter over 24 months.
 *
 * Returns 0..1000. *band_out (optional) receives the band label.
 */
int compass_score_compute(double penny_slope, double vigilance_slope,
                          double creation_slope, double crew_slope,
                          const char **band_out);

#ifdef __cplusplus
}
#endif

#endif /* COMPASS_SCORE_H */
