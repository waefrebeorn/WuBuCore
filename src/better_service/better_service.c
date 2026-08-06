#include "better_service.h"

#include "wubucore.h"

int better_service_run(service_mode mode, int years, service_result *out) {
    if (!out || years < 1) return -1;

    double adoption = 0.02;
    double resentment = 0.0;
    const char *label = (mode == SERVICE_ENFORCE)
                            ? "ENFORCE - fight the alternative"
                            : "SERVE - be the better alternative";

    for (int y = 0; y < years; y++) {
        double growth, defectors;
        if (mode == SERVICE_ENFORCE) {
            growth = 0.06 * (1.0 - adoption);
            resentment += 0.10 + 0.02 * (y + 1);
            defectors = 0.15 * adoption;
        } else {
            growth = 0.45 * adoption * (1.0 - adoption);
            resentment *= 0.85;
            defectors = 0.02 * adoption;
        }
        adoption = wubu_clamp(adoption + growth - defectors, 0.0, 0.99);
        if (resentment < 0.0) resentment = 0.0;
    }

    out->final_adoption = adoption;
    out->final_resentment = resentment;
    out->label = label;
    return 0;
}
