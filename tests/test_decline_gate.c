/* test_decline_gate — GOV-6 Article III's statistical gate,
 * verified against worked numbers from the doctrine. */
#include <stdio.h>
#include <math.h>

#include "decline_gate.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 1e-6)

int main(void) {
    dg_result r;

    /* Doctrine worked case: baseline decline 0.10 +/- 0.03, k=3.
     * A spike to 0.30 is (0.30-0.10)/0.03 = 6.67 sigma -> ANOMALY,
     * process declined (default on). */
    CK(decline_gate(0.10, 0.03, 0.30, 3.0, 1, &r) == 0, "gate ok");
    CK(NEAR(r.z, 6.6666667), "z = 6.67");
    CK(r.anomaly == 1, "spike is anomaly");
    CK(r.process_on == 0, "process declined");

    /* Usual decline 0.10 -> z=0, not anomaly, process on. */
    CK(decline_gate(0.10, 0.03, 0.10, 3.0, 1, &r) == 0, "gate ok 2");
    CK(NEAR(r.z, 0.0), "z = 0");
    CK(r.anomaly == 0, "usual is clear");
    CK(r.process_on == 1, "process happens");

    /* Slight rise 0.13 -> z=1, clear, on. */
    CK(decline_gate(0.10, 0.03, 0.13, 3.0, 1, &r) == 0, "gate ok 3");
    CK(r.anomaly == 0 && r.process_on == 1, "slight rise clear");

    /* Gradual trend 0.07 -> z=-1: a slow baseline shift is NOT an
     * anomaly — the population is becoming the new ethos. On. */
    CK(decline_gate(0.10, 0.03, 0.07, 3.0, 1, &r) == 0, "gate ok 4");
    CK(r.anomaly == 0 && r.process_on == 1, "trend is not anomaly");

    /* Collapse to 0.00 -> z=-3.33 < -3: anomaly (negative side). */
    CK(decline_gate(0.10, 0.03, 0.00, 3.0, 1, &r) == 0, "gate ok 5");
    CK(r.anomaly == 1 && r.process_on == 0, "collapse declined");

    /* Default-off process: even a clear signal stays off. */
    CK(decline_gate(0.10, 0.03, 0.10, 3.0, 0, &r) == 0, "gate ok 6");
    CK(r.anomaly == 0 && r.process_on == 0, "default-off stays off");

    /* Zero-variation baseline: any deviation is an anomaly. */
    CK(decline_gate(0.10, 0.0, 0.11, 3.0, 1, &r) == 0, "gate ok 7");
    CK(r.anomaly == 1, "zero-std deviation is anomaly");

    /* The outlier-vs-trend distinction, batch: same baseline, one
     * outlier among trends. */
    double sigs[4] = {0.10, 0.09, 0.30, 0.08};
    dg_result rs[4];
    CK(decline_gate_batch(0.10, 0.03, 3.0, 1, sigs, 4, rs) == 0,
       "batch ok");
    CK(rs[0].process_on == 1, "usual on");
    CK(rs[1].process_on == 1, "trend on");
    CK(rs[2].process_on == 0, "outlier off");
    CK(rs[3].process_on == 1, "trend2 on");

    /* Bad args: negative std, k <= 0. */
    CK(decline_gate(0.10, -1.0, 0.10, 3.0, 1, &r) == -1, "neg std");
    CK(decline_gate(0.10, 0.03, 0.10, 0.0, 1, &r) == -1, "k <= 0");

    printf(fails ? "%d FAILURES\n" : "DECLINE_GATE: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
