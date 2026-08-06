/*
 * mirror_score.h — the entropy audit (GOV-3 Art. VI / RND-18):
 * 0 = pure tool, 100 = pure mirror.
 */
#ifndef MIRROR_SCORE_H
#define MIRROR_SCORE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mirror_result {
    double score;       /* 0..100 */
    const char *verdict; /* static verdict label */
} mirror_result;

/* Counts: agree, disagree, novel, confirm, comfort. */
mirror_result mirror_score_compute(int agree, int disagree, int novel,
                                   int confirm, int comfort);

#ifdef __cplusplus
}
#endif

#endif /* MIRROR_SCORE_H */
