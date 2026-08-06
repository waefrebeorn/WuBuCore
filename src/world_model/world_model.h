/*
 * world_model.h — the unified aggregation engine (RND-17):
 * mean / median / skill-weighted / recalibrated / hybrid.
 */
#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aggregate_result {
    double mean;
    double median;
    double skill_weighted;
    double recalibrated;
    size_t n;
} aggregate_result;

/* Aggregate forecasts[0..n); skills optional (NULL = unweighted). */
aggregate_result world_model_aggregate(const double *forecasts,
                                       const double *skills, size_t n);

/* Human + silicon blend (SAGE pattern). */
double world_model_hybrid(const aggregate_result *human,
                          const aggregate_result *silicon,
                          double human_weight);

/* Robust median of an array (does not modify input). */
double world_model_median(const double *v, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* WORLD_MODEL_H */
