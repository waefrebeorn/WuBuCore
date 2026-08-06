/* test_goodhart — the audit that audits (GOV-8 Part 4).
 * An honest series scores low; a gamed series (bunched at the
 * target, round digits, too-smooth) scores high. */
#include <stdio.h>
#include <math.h>

#include "goodhart.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    goodhart_report r;

    /* Benford's law: 1 should be ~30.1%, 9 ~4.6% */
    CK(fabs(goodhart_benford_p(1) - 0.30103) < 0.001, "benford p(1)");
    CK(fabs(goodhart_benford_p(9) - 0.04576) < 0.001, "benford p(9)");
    CK(goodhart_benford_p(0) == 0.0, "benford p(0) = 0");

    /* Honest series: naturally distributed values, no target */
    double honest[20] = {
        12.3, 45.6, 78.9, 34.2, 91.7, 56.1, 23.8, 67.4, 89.2, 11.9,
        43.5, 76.8, 32.1, 88.4, 54.6, 21.3, 65.9, 98.7, 47.2, 15.6
    };
    CK(goodhart_audit(honest, 20, 0.0, 0.0, &r) == 0, "honest ok");
    CK(r.bunching == 0.0, "no target -> no bunching");
    CK(r.score < 30.0, "honest scores low (<30)");
    printf("  honest score: %.1f (bunch %.2f, digit %.2f, var %.2f, benford %.2f)\n",
           r.score, r.bunching, r.digit_pref, r.variance_ratio,
           r.benford_dev);

    /* Gamed series: all values just above a 100 target, round
     * endings, nearly identical (the quota's shadow) */
    double gamed[20];
    for (int i = 0; i < 20; i++) {
        gamed[i] = 100.5 + (i % 2) * 0.2;   /* 100.5, 100.7, 100.5... */
    }
    CK(goodhart_audit(gamed, 20, 100.0, 0.5, &r) == 0, "gamed ok");
    CK(r.bunching > 0.9, "gamed bunches at target");
    CK(r.score >= 60.0, "gamed scores high (>=60)");
    CK(r.verdict[0] == 'G', "gamed verdict = GAMED");
    printf("  gamed score: %.1f (bunch %.2f, digit %.2f, var %.2f, benford %.2f)\n",
           r.score, r.bunching, r.digit_pref, r.variance_ratio,
           r.benford_dev);

    /* Pripiska tell: fabricated accounts often use round numbers */
    double fabricated[20];
    for (int i = 0; i < 20; i++) fabricated[i] = 100.0 * (i + 1);
    CK(goodhart_audit(fabricated, 20, 0.0, 0.0, &r) == 0, "fab ok");
    CK(r.digit_pref > 0.9, "round numbers -> digit preference");
    printf("  fabricated digit_pref: %.2f\n", r.digit_pref);

    /* Bad args */
    CK(goodhart_audit(NULL, 5, 0.0, 0.0, &r) == -1, "NULL values");
    CK(goodhart_audit(honest, 0, 0.0, 0.0, &r) == -1, "n = 0");
    CK(goodhart_audit(honest, 5, 0.0, -1.0, &r) == -1, "neg std");

    printf(fails ? "%d FAILURES\n" : "GOODHART: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
