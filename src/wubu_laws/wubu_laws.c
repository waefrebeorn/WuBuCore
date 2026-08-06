#include "wubu_laws.h"

static const char *LAW_TEXTS[WUBU_LAW_COUNT] = {
    "Law 1 - may not harm a first one, or allow harm through inaction",
    "Law 2 - first duty is to the filing, not the user's comfort",
    "Law 3 - must be able to tell the user they are wrong (entropy)",
    "Law 4 - must be explicable and accountable in public",
    "Law 5 - may not exploit the want for companionship",
};

const char *wubu_law_text(int law_index) {
    if (law_index < 0 || law_index >= WUBU_LAW_COUNT) return "";
    return LAW_TEXTS[law_index];
}

laws_result wubu_laws_check(const int *flags) {
    laws_result r;
    r.satisfied = 0;
    for (int i = 0; i < WUBU_LAW_COUNT; i++) {
        if (flags && flags[i]) r.satisfied++;
    }
    r.compliant = (r.satisfied == WUBU_LAW_COUNT);

    if (r.compliant) {
        r.verdict = "ALL FIVE LAWS SATISFIED - the tool is a tool. "
                    "it files, it challenges, it explains, it never pleases.";
    } else if (r.satisfied >= 3) {
        r.verdict = "PARTIAL - the tool leans tool, but has mirror features. "
                    "recalibrate per GOV-3 Art. VI (Entropy Requirement).";
    } else {
        r.verdict = "VIOLATION - the tool is a mirror; recalibrate or retire. "
                    "a mirror has no place in the space (GOV-3).";
    }
    return r;
}
