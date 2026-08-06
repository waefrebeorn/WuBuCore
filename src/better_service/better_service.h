/*
 * better_service.h — the Newell model (ECO-7): enforcement vs
 * better service over N years.
 */
#ifndef BETTER_SERVICE_H
#define BETTER_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum service_mode {
    SERVICE_ENFORCE = 0, /* fight the alternative */
    SERVICE_SERVE        /* be the better alternative */
} service_mode;

typedef struct service_result {
    double final_adoption;   /* 0..1 */
    double final_resentment; /* 0.. */
    const char *label;       /* static mode label */
} service_result;

int better_service_run(service_mode mode, int years, service_result *out);

#ifdef __cplusplus
}
#endif

#endif /* BETTER_SERVICE_H */
