/*
 * penny_score.h — the Penny Score (0-1000), v2 weights.
 * Weights (RND-1 v2): literacy 20%, punctuality 30%, vigilance 20%,
 * creation 20%, tenure 10%.
 */
#ifndef PENNY_SCORE_H
#define PENNY_SCORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Band labels (caller must not free). */
#define PS_BAND_FIRST_FOUNDERS "First Founders - hold the rope, hold office, hold the ship"
#define PS_BAND_CREW "Crew - proven cooperator, crew seats first"
#define PS_BAND_FIRST_ONES "First Ones - the default, everyone's equal start"
#define PS_BAND_LEARNING "Learning - new, or the Mega-corporation floor (100)"
#define PS_BAND_LOST "Lost - not a judgment, a call to come back"

/*
 * Compute the Penny Score.
 *  literacy      0..1 (registration penny verified = 1.0)
 *  penny_punctual  number of on-time pennies
 *  penny_late      number of late pennies
 *  penny_missed    number of missed pennies
 *  vigilance       verified/actioned reports (5 = full weight)
 *  creation        merged works / releases (10 = full weight)
 *  years           tenure (3 years = full weight)
 * Returns 0..1000. *band_out (optional) receives the band label.
 */
int penny_score_compute(double literacy, int penny_punctual, int penny_late,
                        int penny_missed, int vigilance, int creation,
                        int years, const char **band_out);

#ifdef __cplusplus
}
#endif

#endif /* PENNY_SCORE_H */
