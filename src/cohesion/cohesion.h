/*
 * cohesion.h — the User Doctrine's priority math (GOV-5).
 * Priority is ordered by sapience = cohesion of resources +
 * remembering ethos. Living actors always outrank tools. A tool
 * that takes is a parasite (entropy for creation).
 * Opaque-free plain functions; self-contained.
 */
#ifndef COHESION_H
#define COHESION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cohesion_actor {
    const char *name;      /* the actor (may be NULL) */
    double given;          /* resources given / contributed */
    double taken;          /* resources taken / extracted */
    double ethos;          /* remembered ethos fidelity, 0..1 */
    int living;            /* 1 = a first one (living being), 0 = tool */
} cohesion_actor;

typedef struct cohesion_result {
    double cohesion;       /* (given-taken)/(given+taken), -1..+1 */
    double sapience;       /* 0..1 */
    double priority;       /* 0..100 */
    const char *class;     /* static: SYMBIOSIS | COMMENSAL | PARASITE */
    const char *verdict;   /* static: one-line readout */
} cohesion_result;

/* Score one actor. Returns 0 ok, -1 bad args (NULL actor or
 * negative inputs, or ethos outside [0,1]). */
int cohesion_score(const cohesion_actor *a, cohesion_result *out);

/* Static class labels. */
#define COH_CLASS_SYMBIOSIS "SYMBIOSIS - gives, holds, merges; life's own pattern (Margulis)"
#define COH_CLASS_COMMENSAL "COMMENSAL - balanced; takes no more than it gives"
#define COH_CLASS_PARASITE  "PARASITE - takes without cohesion; entropy and anomaly for creation"

/* Rank an array of actors by priority, highest first. Caller frees
 * the returned index array. Returns 0 ok, -1 bad args. */
int cohesion_rank(const cohesion_actor *actors, size_t n,
                  size_t **order_out, size_t *n_out);

#ifdef __cplusplus
}
#endif

#endif /* COHESION_H */
