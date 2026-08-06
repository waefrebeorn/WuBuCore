/*
 * greed_index.h — the Greed Index (ECO-8): Capture + Nexus +
 * Malice (0-300). Published, never enforced.
 */
#ifndef GREED_INDEX_H
#define GREED_INDEX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct greed_result {
    double total;          /* 0..300 */
    const char *band;      /* static band label */
    const char *response;  /* static response label */
} greed_result;

#define GREED_BAND_FILED "FILED - observed, logged, watched with love"
#define GREED_BAND_WATCHED "WATCHED - spotlight intensifies; WBI-1 monitoring"
#define GREED_BAND_FLAGGED "FLAGGED - the ledger publishes; illegal action goes to the Bureau"

/* Compute the index. capture/nexus/malice in 0..100. */
greed_result greed_index_compute(double capture, double nexus, double malice);

#ifdef __cplusplus
}
#endif

#endif /* GREED_INDEX_H */
