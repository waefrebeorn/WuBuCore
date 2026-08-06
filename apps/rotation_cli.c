/* rotation_cli — term/age/election check + ratchet test (GOV-9). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rotation.h"

int main(int argc, char **argv) {
    /* usage: rotation office BIRTH_YEAR MIN_AGE MAX_AGE MAX_TERMS
     *                TERMS_SERVED TERM_YEARS YEARS_SINCE_ELECTION
     *        rotation ratchet MAX_ONE_WAY v1 v2 ... */
    if (argc == 9 && strcmp(argv[1], "office") == 0) {
        rotation_office o;
        memset(&o, 0, sizeof(o));
        o.birth_year = atof(argv[2]);
        o.min_age = atof(argv[3]);
        o.max_age = atof(argv[4]);
        o.max_terms = atoi(argv[5]);
        o.terms_served = atoi(argv[6]);
        o.term_seconds = (long)(atof(argv[7]) * 365.0 * 86400.0);
        o.last_election = time(NULL)
                          - (long)(atof(argv[8]) * 365.0 * 86400.0);
        rotation_result r;
        if (rotation_check(&o, time(NULL), &r) != 0) {
            fprintf(stderr, "bad office args\n");
            return 2;
        }
        double secs_to = difftime(r.next_election, time(NULL));
        printf("ROTATION CHECK (GOV-9, real-time clock):\n");
        printf("  terms left     : %d\n", r.terms_left);
        printf("  age ok         : %s\n", r.age_ok ? "yes" : "NO");
        printf("  office ok      : %s\n", r.office_ok ? "yes" : "NO");
        printf("  next election  : in %.0f days\n",
               secs_to / 86400.0);
        printf("  reason         : %s\n", r.reason);
        return 0;
    }
    if (argc >= 4 && strcmp(argv[1], "ratchet") == 0) {
        int max_one_way = atoi(argv[2]);
        int n = argc - 3;
        double *v = (double *)malloc(n * sizeof(double));
        if (!v) return 1;
        for (int i = 0; i < n; i++) v[i] = atof(argv[i + 3]);
        int r = rotation_ratchet(v, (size_t)n, (size_t)max_one_way);
        printf("RATCHET TEST (GOV-9): %d samples, max one-way %d\n",
               n, max_one_way);
        if (r == 1) {
            printf("  RATCHETED: the policy clicks one way too long; "
                   "the Bureau files it\n");
        } else {
            printf("  no ratchet: reversals keep the machine honest\n");
        }
        free(v);
        return 0;
    }
    fprintf(stderr,
            "usage: rotation office BIRTH_YEAR MIN_AGE MAX_AGE "
            "MAX_TERMS TERMS_SERVED TERM_YEARS YEARS_SINCE_ELECTION\n"
            "       rotation ratchet MAX_ONE_WAY v1 v2 ...\n");
    return 2;
}
