/* vigilance_cli — WBI-1 triage (GOV-11, WuBuIntel). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vigilance.h"

int main(int argc, char **argv) {
    /* usage: vigilance CLASS(0-5) SEVERITY NAMES_OFFICIAL CAPACITY SPIKE_Z
     * classes: 0 power, 1 asymmetry, 2 fear-lie, 3 above-license,
     *          4 penny-evasion, 5 other */
    if (argc == 6) {
        vigilance_report r;
        vigilance_result o;
        r.cls = (vigilance_class)atoi(argv[1]);
        r.severity = atof(argv[2]);
        r.names_official = atoi(argv[3]);
        r.capacity = atof(argv[4]);
        r.spike_z = atof(argv[5]);
        if (vigilance_triage(&r, &o) != 0) {
            fprintf(stderr, "bad report args\n");
            return 2;
        }
        printf("WBI-1 TRIAGE (GOV-11 Bureau):\n");
        printf("  class    : %s\n", o.cls_name);
        printf("  severity : %.0f%%\n", r.severity * 100.0);
        printf("  priority : %.1f/100\n", o.priority);
        printf("  escalate : %d (%s)\n", o.escalation,
               o.escalation == 2 ? "Bureau" :
               o.escalation == 1 ? "one level" : "resolve here");
        printf("  verdict  : %s\n", o.verdict);
        return 0;
    }
    fprintf(stderr, "usage: vigilance CLASS(0-5) SEVERITY NAMES_OFFICIAL "
                    "CAPACITY SPIKE_Z\n");
    return 2;
}
