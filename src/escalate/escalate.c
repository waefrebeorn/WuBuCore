#include "escalate.h"

#include <math.h>

int escalate_decide(const escalate_filing *f, escalate_result *out) {
    if (!f || !out || f->complexity < 0.0 || f->complexity > 1.0 ||
        f->capacity < 0.0 || f->capacity > 1.0 ||
        f->spike_k <= 0.0) {
        return -1;
    }

    if (f->names_official) {
        /* the watchers must watch the watcher */
        out->level = ESC_ESCALATE_ALL;
        out->verdict = "ESCALATE ALL: the filing names an official; "
                       "the Bureau must watch the watcher (GOV-7)";
    } else if (f->complexity > f->capacity ||
               (f->spike_z > 0.0 && fabs(f->spike_z) > f->spike_k)) {
        /* too big for the compartment, or a statistical spike */
        out->level = ESC_ESCALATE_ONE;
        out->verdict = "ESCALATE ONE: beyond the compartment's "
                       "capacity (or a spike); pass it up the "
                       "ladder (GOV-7)";
    } else {
        out->level = ESC_RESOLVE;
        out->verdict = "RESOLVE LOWEST: within the compartment's "
                       "grasp; resolve here (GOV-7, Fengqiao)";
    }
    return 0;
}
