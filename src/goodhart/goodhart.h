/*
 * goodhart.h — the audit that audits (GOV-8 Part 4).
 * Goodhart's Law: when a measure becomes a target it ceases to be
 * a good measure. This module audits a set of reported values for
 * the signatures of gaming: threshold bunching, digit preference,
 * variance collapse, and Benford deviation. Score 0 = honest,
 * 100 = gamed. The space runs every measure through it —
 * including the measures that score the measures.
 */
#ifndef GOODHART_H
#define GOODHART_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct goodhart_report {
    double bunching;      /* 0..1: fraction of values within 1% of target */
    double digit_pref;    /* 0..1: fraction of values ending in 0 or 5 */
    double variance_ratio;/* reported std / expected std (1.0 = honest) */
    double benford_dev;   /* 0..1: first-digit deviation from Benford */
    double score;         /* 0..100: overall gaming score */
    const char *verdict;  /* static: one-line readout */
} goodhart_report;

/* Audit an array of reported values. target is the measure's
 * threshold (0 = no threshold). expected_std is the honest
 * variability (0 = auto-estimate). Returns 0 ok, -1 bad args. */
int goodhart_audit(const double *values, size_t n, double target,
                   double expected_std, goodhart_report *out);

/* Benford's law expected frequency for leading digit d (1..9). */
double goodhart_benford_p(int d);

#ifdef __cplusplus
}
#endif

#endif /* GOODHART_H */
