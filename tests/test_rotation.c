/* test_rotation — GOV-9's rotation math: term limits, age limits,
 * the ratchet test, all on the real-time clock. */
#include <stdio.h>
#include <time.h>

#include "rotation.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    time_t now = time(NULL);
    rotation_office o;
    rotation_result r;

    /* Office: 4-year terms, max 2 terms, served 1, age 25..70,
     * holder born 1985. Election 2 years ago. */
    o.last_election = now - 2L * 365L * 86400L;   /* 2 years ago */
    o.term_seconds = 4L * 365L * 86400L;
    o.max_terms = 2;
    o.terms_served = 1;
    o.min_age = 25.0;
    o.max_age = 70.0;
    o.birth_year = 1985.0;

    CK(rotation_check(&o, now, &r) == 0, "check ok");
    CK(r.terms_left == 1, "one term left");
    CK(r.age_ok == 1, "age ok (41)");
    CK(r.office_ok == 1, "office may be held");
    CK(r.next_election > now, "next election in the future");

    /* Term-limited: served both terms. */
    o.terms_served = 2;
    CK(rotation_check(&o, now, &r) == 0, "check ok 2");
    CK(r.terms_left == 0, "no terms left");
    CK(r.office_ok == 0, "office must rotate");
    CK(r.reason[0] == 'T', "reason = TERM-LIMITED");
    o.terms_served = 1;

    /* Age-limited: holder born 1950 -> age ~76 > 70. */
    o.birth_year = 1950.0;
    CK(rotation_check(&o, now, &r) == 0, "check ok 3");
    CK(r.age_ok == 0, "age bound exceeded");
    CK(r.office_ok == 0, "office refused on age");
    CK(r.reason[0] == 'A', "reason = AGE-BOUND");
    o.birth_year = 1985.0;

    /* Age-limited young: min 25, born 2010 -> age ~16. */
    o.birth_year = 2010.0;
    CK(rotation_check(&o, now, &r) == 0, "check ok 4");
    CK(r.age_ok == 0, "too young");
    o.birth_year = 1985.0;

    /* The ratchet test: a policy clicking one way for 5 rotations
     * with max_one_way=2 IS a ratchet. */
    double ratchet[6] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    CK(rotation_ratchet(ratchet, 6, 2) == 1, "ratchet detected");

    /* A policy that reverses is NOT a ratchet. */
    double healthy[6] = {0.1, 0.2, 0.15, 0.25, 0.2, 0.3};
    CK(rotation_ratchet(healthy, 6, 2) == 0, "reversal = no ratchet");

    /* A short one-way run under the limit is fine. */
    double short_run[4] = {0.1, 0.2, 0.3, 0.2};
    CK(rotation_ratchet(short_run, 4, 2) == 0, "short run ok");

    /* Exactly at the limit. */
    double at_limit[4] = {0.1, 0.2, 0.3, 0.35};   /* run=3 > 2 */
    CK(rotation_ratchet(at_limit, 4, 2) == 1, "at limit ratchets");

    /* Bad args. */
    CK(rotation_check(NULL, now, &r) == -1, "NULL office");
    CK(rotation_ratchet(NULL, 5, 2) == -1, "NULL policy");
    CK(rotation_ratchet(ratchet, 1, 2) == -1, "n < 2");
    CK(rotation_ratchet(ratchet, 5, 0) == -1, "max_one_way 0");

    printf(fails ? "%d FAILURES\n" : "ROTATION: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
