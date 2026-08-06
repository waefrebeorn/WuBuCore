#include "vigilance.h"

#include <math.h>

const char *vigilance_class_name(vigilance_class c) {
    switch (c) {
    case VIG_POWER_CONCENTRATION:
        return "POWER CONCENTRATION - power concentrating without check";
    case VIG_RULE_ASYMMETRY:
        return "RULE ASYMMETRY - a rule for one party, not another";
    case VIG_FEAR_LIE:
        return "FEAR LIE - a lie told to make someone afraid";
    case VIG_ABOVE_LICENSE:
        return "ABOVE LICENSE - an authority claiming to be above the law";
    case VIG_PENNY_EVASION:
        return "PENNY EVASION - the Penny Clause evaded (digital pennies!)";
    case VIG_OTHER:
    default:
        return "OTHER - everything else, still filed";
    }
}

int vigilance_triage(const vigilance_report *r, vigilance_result *out) {
    if (!r || !out || r->severity < 0.0 || r->severity > 1.0 ||
        r->capacity < 0.0 || r->capacity > 1.0) {
        return -1;
    }

    out->cls_name = vigilance_class_name(r->cls);

    /* Base priority: class weight + severity. Penny evasion is the
     * space's special sin (the license's through-line). */
    double class_weight;
    switch (r->cls) {
    case VIG_PENNY_EVASION:      class_weight = 1.0; break;
    case VIG_ABOVE_LICENSE:      class_weight = 0.9; break;
    case VIG_POWER_CONCENTRATION:class_weight = 0.8; break;
    case VIG_FEAR_LIE:           class_weight = 0.7; break;
    case VIG_RULE_ASYMMETRY:     class_weight = 0.6; break;
    case VIG_OTHER:
    default:                     class_weight = 0.4; break;
    }
    out->priority = (class_weight * 60.0) + (r->severity * 40.0);
    if (out->priority > 100.0) out->priority = 100.0;

    /* Escalation: the Fengqiao ladder (GOV-7). A report naming an
     * official always reaches the Bureau. */
    if (r->names_official) {
        out->escalation = 2;
        out->verdict = "ESCALATE ALL: the report names an official; "
                       "the Bureau must watch the watcher (GOV-7)";
    } else if (r->severity > r->capacity ||
               (r->spike_z > 0.0 && fabs(r->spike_z) > 3.0)) {
        out->escalation = 1;
        out->verdict = "ESCALATE ONE: beyond the compartment (or a "
                       "spike); the division files it (GOV-7)";
    } else {
        out->escalation = 0;
        out->verdict = "RESOLVE LOWEST: within the compartment; the "
                       "grid worker files it (GOV-7)";
    }
    return 0;
}
