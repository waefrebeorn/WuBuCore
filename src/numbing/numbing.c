#include "numbing.h"

#include "wubucore.h"

int numbing_run(numbing_world world, int gens, numbing_result *out) {
    if (!out || gens < 1) return -1;

    double stress, protection;
    const char *label;
    switch (world) {
    case NUMBING_CALM:
        stress = 0.06; protection = 0.85;
        label = "CALM - low stress, protected (omega-3, work, sleep)";
        break;
    case NUMBING_MODERN:
        stress = 0.16; protection = 0.55;
        label = "MODERN - four inflations, partial protection";
        break;
    default:
        stress = 0.26; protection = 0.30;
        label = "NEXUS - pacifier economy, accelerated stress";
        break;
    }

    double sensitivity = 1.0;
    for (int g = 0; g < gens; g++) {
        double erosion = stress * (1.0 - protection);
        sensitivity = wubu_clamp(sensitivity - erosion, 0.05, 1.0);
    }

    out->final_sensitivity = sensitivity;
    out->numbed = sensitivity < 0.5;
    out->label = label;
    return 0;
}
