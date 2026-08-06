/* goodhart_cli — audit reported values for gaming signatures
 * (GOV-8 Part 4). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "goodhart.h"

int main(int argc, char **argv) {
    /* usage: goodhart TARGET EXPECTED_STD v1 v2 v3 ...
     *        goodhart --benford  (print Benford's law) */
    if (argc >= 2 && strcmp(argv[1], "--benford") == 0) {
        printf("Benford's law (first-digit frequencies):\n");
        for (int d = 1; d <= 9; d++) {
            printf("  %d: %.1f%%\n", d, 100.0 * goodhart_benford_p(d));
        }
        return 0;
    }
    if (argc >= 5) {
        double target = atof(argv[1]);
        double estd = atof(argv[2]);
        int n = argc - 3;
        double *v = (double *)malloc(n * sizeof(double));
        if (!v) return 1;
        for (int i = 0; i < n; i++) v[i] = atof(argv[i + 3]);
        goodhart_report r;
        if (goodhart_audit(v, (size_t)n, target, estd, &r) != 0) {
            fprintf(stderr, "bad audit args\n");
            free(v);
            return 2;
        }
        printf("GAMING AUDIT (GOV-8): %d values, target %.1f\n", n, target);
        printf("  threshold bunching : %.0f%%  (values within 1%% of target)\n",
               100.0 * r.bunching);
        printf("  digit preference   : %.0f%%  (values ending 0 or 5)\n",
               100.0 * r.digit_pref);
        printf("  variance ratio     : %.2f  (1.0 = honest spread)\n",
               r.variance_ratio);
        printf("  benford deviation  : %.0f%%\n", 100.0 * r.benford_dev);
        printf("  GAMING SCORE       : %.1f/100\n", r.score);
        printf("  verdict            : %s\n", r.verdict);
        free(v);
        return 0;
    }
    fprintf(stderr, "usage: goodhart TARGET EXPECTED_STD v1 v2 ...\n"
                    "       goodhart --benford\n");
    return 2;
}
