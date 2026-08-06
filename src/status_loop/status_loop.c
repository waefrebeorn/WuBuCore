#include "status_loop.h"

#include "wubucore.h"

const char *status_loop_label(status_world world) {
    switch (world) {
    case STATUS_SCARCITY:
        return "SCARCITY - rationed marker; class locked in forever";
    case STATUS_ABUNDANCE:
        return "ABUNDANCE - technology cheapens the marker; class fades";
    default:
        return "FEEDBACK - abundance + embraced loop; status dies fastest";
    }
}

int status_loop_run(status_world world, int years, status_result *out) {
    if (!out || years < 1) return -1;

    double adoption = 0.01;
    double status = 1.0;

    for (int y = 0; y < years; y++) {
        double growth, decay;
        if (world == STATUS_SCARCITY) {
            double target = 0.10;
            growth = (target - adoption) * 0.2;
            decay = 0.0;
        } else if (world == STATUS_ABUNDANCE) {
            growth = 0.25 * adoption * (1.0 - adoption);
            decay = 0.02 * adoption;
        } else {
            growth = 0.30 * adoption * (1.0 - adoption);
            decay = 0.10 * adoption * (1.0 + status);
        }
        adoption = wubu_clamp(adoption + growth, 0.0, 1.0);
        status = wubu_clamp(status - decay, 0.0, 1.0);
    }

    out->final_adoption = adoption;
    out->final_status = status;
    out->status_dead = status < 0.05;
    return 0;
}
