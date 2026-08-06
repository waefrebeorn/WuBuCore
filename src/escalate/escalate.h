/*
 * escalate.h — the Fengqiao ladder (GOV-7 §1.7, GOV-7 Part 2).
 * Resolve lowest -> escalate one -> escalate all. A filing that
 * names an official always escalates to the Bureau: the watchers
 * must watch the watcher. The must is math.
 */
#ifndef ESCALATE_H
#define ESCALATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct escalate_filing {
    double complexity;      /* the filing's complexity, 0..1 */
    double capacity;        /* the compartment's capacity, 0..1 */
    int names_official;     /* 1 if the filing names a Bureau member */
    double spike_z;         /* the filing's decline_gate z-score
                             * (0 = no spike signal) */
    double spike_k;         /* the space's spike threshold (3.0) */
} escalate_filing;

typedef struct escalate_result {
    int level;              /* 0 = resolve lowest, 1 = escalate one,
                             * 2 = escalate all (Bureau) */
    const char *verdict;    /* static: one-line readout */
} escalate_result;

/* Escalation levels. */
#define ESC_RESOLVE 0
#define ESC_ESCALATE_ONE 1
#define ESC_ESCALATE_ALL 2

/* Decide the escalation level. Returns 0 ok, -1 bad args. */
int escalate_decide(const escalate_filing *f, escalate_result *out);

#ifdef __cplusplus
}
#endif

#endif /* ESCALATE_H */
