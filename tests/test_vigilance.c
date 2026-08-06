/* test_vigilance — the Bureau's WBI-1 triage (GOV-11). */
#include <stdio.h>
#include <string.h>

#include "vigilance.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    vigilance_report r;
    vigilance_result o;

    /* Penny evasion: the space's special sin, loud severity. */
    r.cls = VIG_PENNY_EVASION; r.severity = 0.8; r.names_official = 0;
    r.capacity = 0.9; r.spike_z = 0.0;
    CK(vigilance_triage(&r, &o) == 0, "triage ok");
    CK(o.priority > 90.0, "penny evasion priority high");
    CK(strcmp(o.cls_name, "PENNY EVASION - the Penny Clause evaded (digital pennies!)") == 0,
       "class name exact");
    CK(o.escalation == 0, "resolves in compartment");

    /* Above license, names an official: always escalate all. */
    r.cls = VIG_ABOVE_LICENSE; r.severity = 0.3; r.names_official = 1;
    CK(vigilance_triage(&r, &o) == 0, "triage ok 2");
    CK(o.escalation == 2, "names official -> escalate all");
    CK(strstr(o.verdict, "watch the watcher") != NULL, "watcher verdict");

    /* Severity above capacity: escalate one. */
    r.cls = VIG_POWER_CONCENTRATION; r.severity = 0.95;
    r.names_official = 0; r.capacity = 0.5;
    CK(vigilance_triage(&r, &o) == 0, "triage ok 3");
    CK(o.escalation == 1, "beyond capacity -> escalate one");

    /* Statistical spike: escalate one. */
    r.cls = VIG_RULE_ASYMMETRY; r.severity = 0.4; r.capacity = 0.9;
    r.spike_z = 5.0;
    CK(vigilance_triage(&r, &o) == 0, "triage ok 4");
    CK(o.escalation == 1, "spike -> escalate one");

    /* Class weights order: penny > above-license > power > fear >
     * asymmetry > other (same severity). */
    double pri[6];
    vigilance_report rr;
    for (int c = 0; c < 6; c++) {
        rr.cls = (vigilance_class)c; rr.severity = 0.5;
        rr.names_official = 0; rr.capacity = 0.9; rr.spike_z = 0.0;
        vigilance_triage(&rr, &o);
        pri[c] = o.priority;
    }
    CK(pri[VIG_PENNY_EVASION] > pri[VIG_ABOVE_LICENSE], "penny > above");
    CK(pri[VIG_ABOVE_LICENSE] > pri[VIG_POWER_CONCENTRATION], "above > power");
    CK(pri[VIG_POWER_CONCENTRATION] > pri[VIG_FEAR_LIE], "power > fear");
    CK(pri[VIG_FEAR_LIE] > pri[VIG_RULE_ASYMMETRY], "fear > asymmetry");
    CK(pri[VIG_RULE_ASYMMETRY] > pri[VIG_OTHER], "asymmetry > other");

    /* Bad args. */
    CK(vigilance_triage(NULL, &o) == -1, "NULL report");
    r.severity = 1.5;
    CK(vigilance_triage(&r, &o) == -1, "severity > 1");

    printf(fails ? "%d FAILURES\n" : "VIGILANCE: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
