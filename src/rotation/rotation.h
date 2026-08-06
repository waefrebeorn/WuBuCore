/*
 * rotation.h — the Ratchet Doctrine's rotation math (GOV-9).
 * Term limits, age limits, local elections, and the ratchet test,
 * all computed from the real-time clock (time(2)). The clock is
 * the constitution: it cannot be lobbied, extended, or gamed.
 */
#ifndef ROTATION_H
#define ROTATION_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rotation_office {
    time_t last_election;  /* when the last election was held (epoch) */
    long term_seconds;     /* term length in seconds */
    int max_terms;         /* hard term limit (0 = unlimited) */
    int terms_served;      /* terms already served */
    double min_age;        /* minimum age in years (0 = none) */
    double max_age;        /* maximum age in years (0 = none) */
    double birth_year;     /* the office holder's birth year (e.g. 1980) */
} rotation_office;

typedef struct rotation_result {
    time_t next_election;  /* when the next election falls */
    int terms_left;        /* terms remaining (0 = term-limited) */
    int age_ok;            /* 1 if within age bounds */
    int office_ok;         /* 1 if the office may be held */
    const char *reason;    /* static: one-line readout */
} rotation_result;

/* Evaluate an office against the real-time clock `now`. Returns
 * 0 ok, -1 bad args. */
int rotation_check(const rotation_office *o, time_t now,
                   rotation_result *out);

/* The ratchet test: given a series of policy positions sampled
 * each rotation, return 1 if the policy has moved one way for
 * more than `max_one_way` consecutive samples without a reversal.
 * Returns 0 (no ratchet), 1 (ratcheted), -1 bad args. */
int rotation_ratchet(const double *policy, size_t n, size_t max_one_way);

#ifdef __cplusplus
}
#endif

#endif /* ROTATION_H */
