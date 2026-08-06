/*
 * numbing.h — the electrical numbing (ECO-6): chronic stress
 * downregulates reward sensitivity across generations.
 */
#ifndef NUMBING_H
#define NUMBING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum numbing_world {
    NUMBING_CALM = 0,   /* low stress, protected */
    NUMBING_MODERN,     /* four inflations, partial protection */
    NUMBING_NEXUS       /* pacifier economy, accelerated stress */
} numbing_world;

typedef struct numbing_result {
    double final_sensitivity; /* 0..1; <0.5 = structurally numbed */
    int numbed;               /* 1 when sensitivity < 0.5 */
    const char *label;        /* static world label */
} numbing_result;

int numbing_run(numbing_world world, int gens, numbing_result *out);

#ifdef __cplusplus
}
#endif

#endif /* NUMBING_H */
