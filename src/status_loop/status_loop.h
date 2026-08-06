/*
 * status_loop.h — the special-feeling feedback loop (ECO-3):
 * scarcity / abundance / feedback worlds. Each simulation is a
 * self-contained module; the world is opaque.
 */
#ifndef STATUS_LOOP_H
#define STATUS_LOOP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum status_world {
    STATUS_SCARCITY = 0,  /* rationed marker -> class persists */
    STATUS_ABUNDANCE,     /* technology-produced marker -> class fades */
    STATUS_FEEDBACK       /* abundance + embraced loop -> status dies */
} status_world;

typedef struct status_result {
    double final_adoption; /* 0..1 */
    double final_status;   /* 0..1, 0 = superiority is boring */
    int status_dead;       /* 1 when status < 0.05 */
} status_result;

/* Run one world for `years` years. Returns 0 ok, -1 bad world. */
int status_loop_run(status_world world, int years, status_result *out);

/* Human-readable world label (static). */
const char *status_loop_label(status_world world);

#ifdef __cplusplus
}
#endif

#endif /* STATUS_LOOP_H */
