/* test_world_model — the verified aggregation numbers. */
#include <stdio.h>
#include <math.h>

#include "world_model.h"
#include "mirror_score.h"
#include "wubu_laws.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 0.01)

int main(void) {
    /* Verified: 0.55 0.60 0.70 0.45 0.80 with skills 3 5 2 4 6:
     * mean 0.620, median 0.600, skill-weighted 0.770, recal 0.703 */
    double f[5] = {0.55, 0.60, 0.70, 0.45, 0.80};
    double sk[5] = {3, 5, 2, 4, 6};
    aggregate_result a = world_model_aggregate(f, sk, 5);
    CK(NEAR(a.mean, 0.620), "mean 0.620");
    CK(NEAR(a.median, 0.600), "median 0.600");
    CK(NEAR(a.skill_weighted, 0.770), "skill-weighted 0.770");
    CK(NEAR(a.recalibrated, 0.703), "recalibrated 0.703");

    /* Hybrid: human 0.703 + silicon 0.600 = 0.651 */
    double sf[5] = {0.52, 0.58, 0.65, 0.50, 0.75};
    aggregate_result sa = world_model_aggregate(sf, NULL, 5);
    CK(NEAR(sa.recalibrated, 0.600), "silicon recal 0.600");
    double h = world_model_hybrid(&a, &sa, 0.5);
    CK(NEAR(h, 0.651), "hybrid 0.651");

    /* Overconfident crowd: 0.90 0.85 0.95 -> recalibrated 0.800 */
    double oc[3] = {0.90, 0.85, 0.95};
    aggregate_result oa = world_model_aggregate(oc, NULL, 3);
    CK(NEAR(oa.recalibrated, 0.800), "overconfident squeezed to 0.800");

    /* mirror_score: companion-AI = 100, tool standard = 40 */
    mirror_result mr = mirror_score_compute(8, 0, 0, 9, 7);
    CK(NEAR(mr.score, 100.0), "mirror 100");
    mr = mirror_score_compute(4, 6, 5, 2, 1);
    CK(mr.score < 50.0, "tool < 50");

    /* wubu_laws: 5/5 compliant, 1/5 violation */
    int good[WUBU_LAW_COUNT] = {1, 1, 1, 1, 1};
    laws_result lr = wubu_laws_check(good);
    CK(lr.compliant == 1, "5/5 compliant");
    int bad[WUBU_LAW_COUNT] = {1, 0, 0, 0, 0};
    lr = wubu_laws_check(bad);
    CK(lr.compliant == 0 && lr.satisfied == 1, "1/5 violation");

    printf(fails ? "%d FAILURES\n" : "WORLD_MODEL/MIRROR/LAWS: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
