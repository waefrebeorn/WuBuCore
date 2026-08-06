/* test_escalate — GOV-7's Fengqiao ladder. */
#include <stdio.h>
#include <string.h>

#include "escalate.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    escalate_filing f;
    escalate_result r;

    /* Resolve lowest: simple filing within capacity, no official,
     * no spike. */
    f.complexity = 0.2; f.capacity = 0.8; f.names_official = 0;
    f.spike_z = 0.5; f.spike_k = 3.0;
    CK(escalate_decide(&f, &r) == 0, "decide ok");
    CK(r.level == ESC_RESOLVE, "resolve lowest");
    CK(r.verdict[0] == 'R', "verdict RESOLVE");

    /* Escalate one: filing exceeds compartment capacity. */
    f.complexity = 0.9; f.capacity = 0.5;
    CK(escalate_decide(&f, &r) == 0, "decide ok 2");
    CK(r.level == ESC_ESCALATE_ONE, "escalate one (capacity)");

    /* Escalate one: statistical spike. */
    f.complexity = 0.2; f.capacity = 0.8; f.spike_z = 6.67;
    CK(escalate_decide(&f, &r) == 0, "decide ok 3");
    CK(r.level == ESC_ESCALATE_ONE, "escalate one (spike)");

    /* Escalate all: names an official, even a trivial filing. */
    f.complexity = 0.05; f.capacity = 0.9; f.spike_z = 0.1;
    f.names_official = 1;
    CK(escalate_decide(&f, &r) == 0, "decide ok 4");
    CK(r.level == ESC_ESCALATE_ALL, "escalate all (official)");
    CK(strstr(r.verdict, "watcher") != NULL, "watcher verdict");

    /* Bad args. */
    CK(escalate_decide(NULL, &r) == -1, "NULL filing");
    f.complexity = 1.5;
    CK(escalate_decide(&f, &r) == -1, "complexity > 1");
    f.complexity = 0.2; f.spike_k = 0.0;
    CK(escalate_decide(&f, &r) == -1, "k <= 0");

    printf(fails ? "%d FAILURES\n" : "ESCALATE: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
