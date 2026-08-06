#include "goodhart.h"

#include <math.h>

double goodhart_benford_p(int d) {
    if (d < 1 || d > 9) return 0.0;
    return log10(1.0 + 1.0 / (double)d);
}

static double leading_digit(double x) {
    if (x <= 0.0 || !isfinite(x)) return 0.0;
    double l = floor(log10(x));
    double mag = pow(10.0, l);
    double lead = x / mag;
    if (lead < 1.0) lead *= 10.0;
    return floor(lead + 1e-9);
}

static double mean_of(const double *v, size_t n) {
    double s = 0.0;
    for (size_t i = 0; i < n; i++) s += v[i];
    return s / (double)n;
}

static double std_of(const double *v, size_t n) {
    if (n < 2) return 0.0;
    double m = mean_of(v, n), s = 0.0;
    for (size_t i = 0; i < n; i++) s += (v[i] - m) * (v[i] - m);
    return sqrt(s / (double)(n - 1));
}

int goodhart_audit(const double *values, size_t n, double target,
                   double expected_std, goodhart_report *out) {
    if (!values || !out || n == 0) return -1;
    if (expected_std < 0.0) return -1;

    /* 1. threshold bunching: values within 1% of the target */
    size_t bunched = 0;
    if (target > 0.0) {
        double tol = target * 0.01;
        for (size_t i = 0; i < n; i++) {
            if (values[i] >= target - tol && values[i] <= target + tol) {
                bunched++;
            }
        }
    }
    out->bunching = target > 0.0 ? (double)bunched / (double)n : 0.0;

    /* 2. digit preference: last digit 0 or 5 (round-number tell) */
    size_t pref = 0;
    for (size_t i = 0; i < n; i++) {
        double x = values[i];
        if (x < 0.0 || !isfinite(x)) continue;
        double frac = x - floor(x);
        if (frac < 0.05 || frac > 0.95) pref++;      /* .00 */
        else if (fabs(frac - 0.5) < 0.05) pref++;    /* .50 */
    }
    out->digit_pref = (double)pref / (double)n;

    /* 3. variance collapse: reported std vs expected (or vs
     * estimate from the data's own spread) */
    double s = std_of(values, n);
    double expected = expected_std > 0.0 ? expected_std
                                         : (s > 0.0 ? s * 1.0 : 1.0);
    out->variance_ratio = (expected > 0.0) ? s / expected : 1.0;

    /* 4. Benford deviation: first-digit distribution vs Benford */
    double benford_err = 0.0;
    {
        double counts[10] = {0};
        size_t total = 0;
        for (size_t i = 0; i < n; i++) {
            double d = leading_digit(values[i]);
            if (d >= 1 && d <= 9) { counts[(int)d]++; total++; }
        }
        if (total > 0) {
            for (int d = 1; d <= 9; d++) {
                double obs = counts[d] / (double)total;
                double exp = goodhart_benford_p(d);
                benford_err += fabs(obs - exp);
            }
            /* max possible error: sum of (1 - benford_p) twice-ish;
             * normalize to 0..1 by dividing by 2 */
            benford_err = benford_err / 2.0;
            if (benford_err > 1.0) benford_err = 1.0;
        }
    }
    out->benford_dev = benford_err;

    /* Score: weighted combination of the four signatures. */
    double s1 = out->bunching * 40.0;                 /* up to 40 */
    double s2 = out->digit_pref * 20.0;               /* up to 20 */
    double s3 = 0.0;
    if (out->variance_ratio < 0.5) s3 = 15.0;         /* collapsed */
    else if (out->variance_ratio < 0.75) s3 = 8.0;    /* tight */
    double s4 = out->benford_dev * 25.0;              /* up to 25 */
    out->score = s1 + s2 + s3 + s4;
    if (out->score > 100.0) out->score = 100.0;

    if (out->score >= 60.0) {
        out->verdict = "GAMED: the measure smells of pripiska — "
                       "flag and re-audit (GOV-8)";
    } else if (out->score >= 30.0) {
        out->verdict = "SUSPECT: some signatures present; watch the "
                       "measure (GOV-8)";
    } else {
        out->verdict = "HONEST: no gaming signatures; the measure "
                       "measures (GOV-8)";
    }
    return 0;
}
