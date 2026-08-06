#include "lifespan_loop.h"

#include <math.h>

static const char *friction_of(double raw) {
    if (raw < 2.0) return "LOW - handoff, not war";
    if (raw < 3.0) return "MODERATE - three generations, three price levels";
    if (raw < 4.0) return "HIGH - four generations queued in one inflated economy";
    return "MAXIMUM - the fullest stack";
}

lifespan_result lifespan_loop_report(double lifespan, double gap) {
    lifespan_result r;
    r.raw_overlap = (gap > 0.0) ? lifespan / gap : 0.0;
    r.generations_alive = (int)floor(r.raw_overlap) + 1;
    r.friction = friction_of(r.raw_overlap);
    return r;
}

int lifespan_loop_timeline(timeline_point *out, size_t n) {
    if (!out || n < 6) return -1;
    const int years[6] = {1900, 1950, 1975, 2000, 2026, 2050};
    const double lifespans[6] = {32.0, 46.0, 59.0, 66.0, 73.0, 78.0};
    for (int i = 0; i < 6; i++) {
        out[i].year = years[i];
        out[i].lifespan = lifespans[i];
        lifespan_result r = lifespan_loop_report(lifespans[i], 25.0);
        out[i].generations_alive = r.generations_alive;
        out[i].friction = r.friction;
    }
    return 0;
}
