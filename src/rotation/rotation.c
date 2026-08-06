#include "rotation.h"

#include <math.h>

/* Approximate seconds per year (365.25 days). */
#define SECS_PER_YEAR 31557600.0

int rotation_check(const rotation_office *o, time_t now,
                   rotation_result *out) {
    if (!o || !out || o->term_seconds <= 0 || o->max_terms < 0 ||
        o->terms_served < 0) {
        return -1;
    }

    out->next_election = o->last_election + o->term_seconds;
    out->terms_left = (o->max_terms > 0)
                          ? o->max_terms - o->terms_served
                          : 1;
    if (out->terms_left < 0) out->terms_left = 0;

    out->age_ok = 1;
    if (o->birth_year > 0.0) {
        /* calendar year = epoch seconds / secs-per-year + 1970 */
        double age = ((double)now / SECS_PER_YEAR) + 1970.0
                     - o->birth_year;
        if (o->min_age > 0.0 && age < o->min_age) out->age_ok = 0;
        if (o->max_age > 0.0 && age > o->max_age) out->age_ok = 0;
    }

    out->office_ok = (out->terms_left > 0 && out->age_ok) ? 1 : 0;

    if (out->terms_left <= 0) {
        out->reason = "TERM-LIMITED: no terms left; rotate now "
                      "(GOV-9 Article III)";
    } else if (!out->age_ok) {
        out->reason = "AGE-BOUND: outside the filed age limits; the "
                      "limit is information, not insult (GOV-9 "
                      "Article II)";
    } else {
        out->reason = "ROTATION-OK: within terms and age; the clock "
                      "runs (GOV-9 Article V)";
    }
    return 0;
}

int rotation_ratchet(const double *policy, size_t n, size_t max_one_way) {
    if (!policy || n < 2 || max_one_way == 0) return -1;

    size_t run = 0;
    int direction = 0;   /* +1 up, -1 down, 0 none */
    for (size_t i = 1; i < n; i++) {
        if (policy[i] > policy[i - 1]) {
            if (direction == 1) {
                run++;
            } else if (direction == -1) {
                return 0;  /* reversal: no ratchet */
            } else {
                direction = 1;
                run = 1;
            }
        } else if (policy[i] < policy[i - 1]) {
            if (direction == -1) {
                run++;
            } else if (direction == 1) {
                return 0;  /* reversal: no ratchet */
            } else {
                direction = -1;
                run = 1;
            }
        }
        if (run > max_one_way) return 1;  /* one-way too long */
    }
    return 0;
}
