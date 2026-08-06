/*
 * filing_check.h — the audit as a tool (GOV-11, AUD-3 fix).
 * The corpus-consistency checks AUD-3 ran by hand-grep, shipped:
 * numbering collision detection, amendment-pointer checks, and
 * dangling-reference detection. The space's audit does not live
 * in a grep; it lives in a tool.
 */
#ifndef FILING_CHECK_H
#define FILING_CHECK_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fc_number {
    const char *number;   /* e.g. "RND-8" */
    const char *title;    /* what this filing claims to be */
} fc_number;

typedef struct fc_report {
    size_t collisions;        /* count of duplicate numbers */
    size_t dangling;          /* count of references with no filing */
    const char *verdict;      /* static: one-line readout */
} fc_report;

/* Check a set of filings for duplicate numbers. filings is an
 * array of {number, title}; n is its length. Returns 0 ok, -1 bad
 * args. Collisions are counted (not listed — the tool files the
 * count; the Bureau reads the file). */
int fc_collisions(const fc_number *filings, size_t n, size_t *collisions);

/* Check that every referenced filing exists. refs is the array of
 * referenced numbers; have is the array of numbers that exist.
 * Returns 0 ok, -1 bad args. dangling counts refs with no match
 * in have. */
int fc_dangling(const char **refs, size_t nrefs,
                const char **have, size_t nhave, size_t *dangling);

/* Amendment-pointer check: does the superseded doc name the
 * superseding doc? Returns 1 if yes, 0 if no, -1 bad args. */
int fc_amended(const char *superseded_text, const char *superseding);

#ifdef __cplusplus
}
#endif

#endif /* FILING_CHECK_H */
