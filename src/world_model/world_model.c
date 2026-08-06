#include "world_model.h"

#include <math.h>
#include <stdlib.h>

#include "wubucore.h"

static int cmp_double(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

double world_model_median(const double *v, size_t n) {
    if (n == 0) return 0.0;
    double *copy = (double *)malloc(n * sizeof(double));
    if (!copy) return 0.0;
    for (size_t i = 0; i < n; i++) copy[i] = v[i];
    qsort(copy, n, sizeof(double), cmp_double);
    double med = (n % 2 == 1) ? copy[n / 2]
                              : (copy[n / 2 - 1] + copy[n / 2]) / 2.0;
    free(copy);
    return med;
}

aggregate_result world_model_aggregate(const double *forecasts,
                                       const double *skills, size_t n) {
    aggregate_result r;
    r.n = n;
    r.mean = 0.0;
    r.median = 0.0;
    r.skill_weighted = 0.0;
    r.recalibrated = 0.0;
    if (n == 0) return r;

    double sum = 0.0;
    for (size_t i = 0; i < n; i++) sum += forecasts[i];
    r.mean = sum / (double)n;
    r.median = world_model_median(forecasts, n);

    if (skills) {
        double wsum = 0.0, vsum = 0.0;
        for (size_t i = 0; i < n; i++) {
            double w = exp(2.0 * skills[i]);
            wsum += w;
            vsum += forecasts[i] * w;
        }
        r.skill_weighted = (wsum > 0.0) ? vsum / wsum : r.mean;
    } else {
        r.skill_weighted = r.mean;
    }

    /* Recalibration: squeeze extremes toward 0.5 (bias correction). */
    double raw = r.skill_weighted;
    if (raw > 0.65) {
        r.recalibrated = 0.5 + (raw - 0.5) * 0.75;
    } else if (raw < 0.35) {
        r.recalibrated = 0.5 - (0.5 - raw) * 0.75;
    } else {
        r.recalibrated = raw;
    }
    return r;
}

double world_model_hybrid(const aggregate_result *human,
                          const aggregate_result *silicon,
                          double human_weight) {
    double hw = wubu_clamp(human_weight, 0.0, 1.0);
    return hw * human->recalibrated + (1.0 - hw) * silicon->recalibrated;
}
