#include "greed_index.h"

#include "wubucore.h"

greed_result greed_index_compute(double capture, double nexus, double malice) {
    greed_result r;
    r.total = wubu_clamp(capture, 0.0, 100.0) +
              wubu_clamp(nexus, 0.0, 100.0) +
              wubu_clamp(malice, 0.0, 100.0);

    if (r.total < 100.0) {
        r.band = GREED_BAND_FILED;
        r.response = "no action - the Index observes and files.";
    } else if (r.total < 200.0) {
        r.band = GREED_BAND_WATCHED;
        r.response = "the spotlight brightens; the actor is invited to file "
                     "(the off-ramp).";
    } else {
        r.band = GREED_BAND_FLAGGED;
        r.response = "the ledger publishes; malicious compliance is "
                     "anticipated - the space answers with better service, "
                     "never force.";
    }
    return r;
}
