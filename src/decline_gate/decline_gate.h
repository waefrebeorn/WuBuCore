/*
 * decline_gate.h — the Integration Doctrine's statistical gate
 * (GOV-6 Article III). Processes are default-on and population-
 * gated: they happen unless the population's decline signal is a
 * statistical anomaly (deviates from its own baseline by more than
 * k sigma). The gate distinguishes outlier from trend — the
 * shepherd's knowing, in math.
 */
#ifndef DECLINE_GATE_H
#define DECLINE_GATE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The gate verdict. */
typedef struct dg_result {
    double z;              /* how many sigma the signal deviates */
    int anomaly;           /* 1 if |z| > k */
    int process_on;        /* the process's final state */
    const char *reason;    /* static: one-line readout */
} dg_result;

/* Decide one gate. mean/std are the population's baseline decline
 * statistics; signal is the current decline reading (0..1 scale,
 * any units); k is the sigma threshold (space default 3.0);
 * default_on is the process's default state. Returns 0 ok, -1 bad
 * args (NULL out, std < 0, k <= 0). */
int decline_gate(double mean, double std, double signal, double k,
                 int default_on, dg_result *out);

/* Batch gate: decide n signals against one baseline. Returns 0 ok,
 * -1 bad args. */
int decline_gate_batch(double mean, double std, double k,
                       int default_on, const double *signals, size_t n,
                       dg_result *outs);

/* The space's default threshold (3 sigma). */
#define DG_K_DEFAULT 3.0

#ifdef __cplusplus
}
#endif

#endif /* DECLINE_GATE_H */
