#include "penny_score.h"

#include "wubucore.h"

static const char *band_of(int score) {
    if (score >= 850) return PS_BAND_FIRST_FOUNDERS;
    if (score >= 700) return PS_BAND_CREW;
    if (score >= 500) return PS_BAND_FIRST_ONES;
    if (score >= 100) return PS_BAND_LEARNING;
    return PS_BAND_LOST;
}

int penny_score_compute(double literacy, int penny_punctual, int penny_late,
                        int penny_missed, int vigilance, int creation,
                        int years, const char **band_out) {
    double lit = wubu_clamp(literacy, 0.0, 1.0);

    int total = penny_punctual + penny_late + penny_missed;
    double punct = (total > 0) ? wubu_clamp((double)penny_punctual / total,
                                            0.0, 1.0)
                               : 0.0;

    double vig = wubu_clamp(vigilance / 5.0, 0.0, 1.0);
    double cre = wubu_clamp(creation / 10.0, 0.0, 1.0);
    double ten = wubu_clamp(years / 3.0, 0.0, 1.0);

    /* v2 weights: 20/30/20/20/10 */
    int score = (int)(lit * 200 + punct * 300 + vig * 200 +
                      cre * 200 + ten * 100 + 0.5);
    if (score > 1000) score = 1000;

    if (band_out) *band_out = band_of(score);
    return score;
}
