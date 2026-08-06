/* test_filing_check — the audit as a tool (GOV-11, AUD-3 fix).
 * The exact defect classes AUD-3 found by hand, now checked by
 * code. */
#include <stdio.h>
#include <string.h>

#include "filing_check.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    /* Collision detection: RND-8 claimed twice (the AUD-1 defect). */
    fc_number filings[4] = {
        {"RND-8", "the Kevin Bacon method"},
        {"RND-8", "the Precedents (ECO-1's claim)"},
        {"GOV-5", "the User Doctrine"},
        {"RND-20", "the Language Doctrine"},
    };
    size_t collisions = 0;
    CK(fc_collisions(filings, 4, &collisions) == 0, "collisions ok");
    CK(collisions == 1, "exactly one collision (RND-8)");

    /* No collisions when clean. */
    fc_number clean[3] = {
        {"RND-1", "scoring"}, {"RND-2", "penny"}, {"RND-3", "chain"},
    };
    CK(fc_collisions(clean, 3, &collisions) == 0, "clean ok");
    CK(collisions == 0, "clean has none");

    /* Dangling: referenced-but-never-filed. */
    const char *refs[5] = {"GOV-1", "GOV-2", "GOV-3", "RND-99", "ECO-1"};
    const char *have[4] = {"GOV-1", "GOV-2", "GOV-3", "ECO-1"};
    size_t dangling = 0;
    CK(fc_dangling(refs, 5, have, 4, &dangling) == 0, "dangling ok");
    CK(dangling == 1, "RND-99 dangles");

    /* Amendment pointers: superseded doc must name the superseder. */
    const char *gov3 = "GOV-3 ... AMENDED BY: GOV-5, GOV-6";
    CK(fc_amended(gov3, "GOV-6") == 1, "amended points to GOV-6");
    CK(fc_amended(gov3, "GOV-99") == 0, "no pointer to GOV-99");
    CK(fc_amended(NULL, "GOV-6") == -1, "NULL text");

    /* The AUD-3 real case: human-centered-doctrine now names GOV-6. */
    const char *fixed_gov3 = "The Human-Centered Doctrine ... AMENDED BY: GOV-5 (User Doctrine) Article I; GOV-6 (Integration Doctrine) Part 3";
    CK(fc_amended(fixed_gov3, "GOV-6") == 1, "AUD-3 fix verified");

    printf(fails ? "%d FAILURES\n" : "FILING_CHECK: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
