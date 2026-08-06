/*
 * lifespan_loop.h — the loop that is not a loop (ECO-5):
 * generations alive = lifespan / generation gap. Friction bands.
 */
#ifndef LIFESPAN_LOOP_H
#define LIFESPAN_LOOP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lifespan_result {
    double raw_overlap;   /* lifespan / gap */
    int generations_alive;
    const char *friction; /* static band label */
} lifespan_result;

/* Compute the stack for a region. */
lifespan_result lifespan_loop_report(double lifespan, double gap);

typedef struct timeline_point {
    int year;
    double lifespan;
    int generations_alive;
    const char *friction;
} timeline_point;

/* The stack over a century of medicine (1900-2050). n out >= 6. */
int lifespan_loop_timeline(timeline_point *out, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* LIFESPAN_LOOP_H */
