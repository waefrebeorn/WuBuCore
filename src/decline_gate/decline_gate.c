#include "decline_gate.h"

#include <math.h>

int decline_gate(double mean, double std, double signal, double k,
                 int default_on, dg_result *out) {
    if (!out || std < 0.0 || k <= 0.0 || mean < 0.0 || signal < 0.0) {
        return -1;
    }
    if (std == 0.0) {
        /* no baseline variation: any deviation is an anomaly */
        out->z = (signal != mean) ? 1e9 : 0.0;
    } else {
        out->z = (signal - mean) / std;
    }
    out->anomaly = (fabs(out->z) > k) ? 1 : 0;
    out->process_on = (default_on && !out->anomaly) ? 1 : 0;

    if (out->anomaly) {
        if (out->z > 0.0) {
            out->reason = "ANOMALY: the population's decline spiked "
                          "past the baseline gate; the process is "
                          "declined (GOV-6 Article III)";
        } else {
            out->reason = "ANOMALY: the signal collapsed past the "
                          "baseline gate; the process is declined for "
                          "review (GOV-6 Article III)";
        }
    } else if (default_on) {
        out->reason = "CLEAR: within the population's baseline; the "
                      "process happens by default (GOV-6 Article III)";
    } else {
        out->reason = "CLEAR: within baseline, but the process is "
                      "default-off; nothing changes (GOV-6 Article III)";
    }
    return 0;
}

int decline_gate_batch(double mean, double std, double k,
                       int default_on, const double *signals, size_t n,
                       dg_result *outs) {
    if (!signals || !outs || std < 0.0 || k <= 0.0) return -1;
    for (size_t i = 0; i < n; i++) {
        if (decline_gate(mean, std, signals[i], k, default_on,
                         &outs[i]) != 0) {
            return -1;
        }
    }
    return 0;
}
