/*
 * crew_score.c — the Crew Score (cooperation metric, 0–1000).
 * Filed RND-1: Crew Score measures how well a first one works
 * with others.  Parody of VantageScore's behavioral model.
 *
 * Weights (RND-1 Part 3):
 *   cooperative acts   30%  — merged PRs with others, joint builds,
 *                              crew holds.  5 acts = full weight.
 *   report integrity   25%  — WBI-1 reports filed that verify
 *                              (honesty signal). 10 reports = full.
 *   responsiveness     20%  — replies to space / Bureau / crew.
 *                              20 replies = full weight.
 *   diversity of mind  15%  — number of mind-countries engaged.
 *                              10 countries = full weight.
 *   longevity          10%  — sustained membership, no gaps.
 *                              24 months = full weight.
 */
#include "crew_score.h"

#include "wubucore.h"

static const char *band_of(int score) {
    if (score >= 850) return CS_BAND_FOUNDERS;
    if (score >= 700) return CS_BAND_CREW;
    if (score >= 500) return CS_BAND_FIRST;
    return CS_BAND_DRIFT;
}

int crew_score_compute(int coop_act, int reports, int replies,
                       int mind_countries, int months_active,
                       const char **band_out) {
    double coop  = wubu_clamp(coop_act / 5.0,    0.0, 1.0);
    double rep   = wubu_clamp(reports / 10.0,    0.0, 1.0);
    double resp  = wubu_clamp(replies / 20.0,    0.0, 1.0);
    double mind  = wubu_clamp(mind_countries / 10.0, 0.0, 1.0);
    double longv = wubu_clamp(months_active / 24.0, 0.0, 1.0);

    /* 30/25/20/15/10 */
    int score = (int)(coop * 300 + rep * 250 + resp * 200 +
                      mind * 150 + longv * 100 + 0.5);
    if (score > 1000) score = 1000;

    if (band_out) *band_out = band_of(score);
    return score;
}
