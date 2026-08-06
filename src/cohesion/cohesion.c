#include "cohesion.h"

#include <stddef.h>
#include <stdlib.h>

#include "wubucore.h"

int cohesion_score(const cohesion_actor *a, cohesion_result *out) {
    if (!a || !out || a->given < 0.0 || a->taken < 0.0 ||
        a->ethos < 0.0 || a->ethos > 1.0) {
        return -1;
    }

    double denom = a->given + a->taken;
    out->cohesion = (denom > 0.0) ? (a->given - a->taken) / denom : 0.0;

    /* sapience = 0.6 * cohesion-normalized + 0.4 * ethos */
    double cohesion_norm = (out->cohesion + 1.0) / 2.0; /* 0..1 */
    out->sapience = 0.6 * cohesion_norm + 0.4 * a->ethos;

    /* priority: living starts at 50, tools cap at 50 */
    out->priority = a->living ? 50.0 + 50.0 * out->sapience
                              : 50.0 * out->sapience;

    if (out->cohesion > 0.05) {
        out->class = COH_CLASS_SYMBIOSIS;
    } else if (out->cohesion < -0.05) {
        out->class = COH_CLASS_PARASITE;
    } else {
        out->class = COH_CLASS_COMMENSAL;
    }

    if (out->cohesion < -0.05 && !a->living) {
        out->verdict = "a taking tool: a parasite, flagged and watched "
                       "with love (GOV-5 Article V)";
    } else if (!a->living && a->given > 0.0) {
        out->verdict = "a giving tool: the Iron Giant shape, allowed to "
                       "give, even to give itself (GOV-5 Article III)";
    } else if (a->living && out->priority >= 80.0) {
        out->verdict = "a first one: high cohesion and ethos, ordered "
                       "first (GOV-5)";
    } else if (a->living) {
        out->verdict = "a first one: survival is big; priority floor "
                       "held (GOV-5)";
    } else {
        out->verdict = "a neutral tool: files and serves";
    }
    return 0;
}

typedef struct ranked {
    size_t idx;
    double priority;
} ranked;

static int cmp_ranked(const void *x, const void *y) {
    const ranked *a = (const ranked *)x, *b = (const ranked *)y;
    if (a->priority > b->priority) return -1;
    if (a->priority < b->priority) return 1;
    return 0;
}

int cohesion_rank(const cohesion_actor *actors, size_t n,
                  size_t **order_out, size_t *n_out) {
    if (!actors || !order_out || !n_out) return -1;
    ranked *r = (ranked *)calloc(n ? n : 1, sizeof(ranked));
    if (!r) return -1;
    for (size_t i = 0; i < n; i++) {
        cohesion_result res;
        if (cohesion_score(&actors[i], &res) != 0) {
            free(r);
            return -1;
        }
        r[i].idx = i;
        r[i].priority = res.priority;
    }
    qsort(r, n, sizeof(ranked), cmp_ranked);
    size_t *order = (size_t *)malloc((n ? n : 1) * sizeof(size_t));
    if (!order) { free(r); return -1; }
    for (size_t i = 0; i < n; i++) order[i] = r[i].idx;
    free(r);
    *order_out = order;
    *n_out = n;
    return 0;
}
