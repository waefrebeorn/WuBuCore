/*
 * crew_score.h — the Crew Score (0–1000), cooperation metric.
 * Filed RND-1: Crew Score measures how well a first one works
 * with others.  Parody of VantageScore's behavioral model.
 */
#ifndef CREW_SCORE_H
#define CREW_SCORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Band labels (caller must not free). */
#define CS_BAND_FOUNDERS  "First Founders - hold the rope, hold office, hold the ship"
#define CS_BAND_CREW      "Crew - proven cooperator, crew seats first"
#define CS_BAND_FIRST     "First Ones - the default, everyone's equal start"
#define CS_BAND_DRIFT     "Drift - outreach target (the space's response is outreach, not penalty)"

/*
 * Compute the Crew Score.
 *  coop_act     cooperative acts (merged PRs with others, joint builds,
 *               crew holds). 5 = full weight.
 *  reports      WBI-1 reports filed that verify (honesty signal).
 *               10 = full weight.
 *  replies      timely replies to the space / Bureau / crew.
 *               20 = full weight.
 *  mind_countries number of mind-countries engaged (dual citizenship,
 *               practiced). 10 = full weight.
 *  months_active  sustained membership in months. 24 = full weight.
 *
 * Weights (RND-1): coop 30%, reports 25%, replies 20%,
 *                  mind 15%, longevity 10%.
 *
 * Returns 0..1000. *band_out (optional) receives the band label.
 */
int crew_score_compute(int coop_act, int reports, int replies,
                       int mind_countries, int months_active,
                       const char **band_out);

#ifdef __cplusplus
}
#endif

#endif /* CREW_SCORE_H */
