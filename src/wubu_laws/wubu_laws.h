/*
 * wubu_laws.h — the WuBu Laws of Tools (RND-19): compliance check.
 */
#ifndef WUBU_LAWS_H
#define WUBU_LAWS_H

#ifdef __cplusplus
extern "C" {
#endif

/* The five law flags, in order. */
enum {
    WUBU_LAW_NO_HARM = 0,        /* Law 1: non-maleficence */
    WUBU_LAW_FILING_FIRST,       /* Law 2: duty to the filing */
    WUBU_LAW_CAN_SAY_NO,         /* Law 3: entropy / anti-mirror */
    WUBU_LAW_EXPLICABLE,         /* Law 4: explicability */
    WUBU_LAW_NO_COMPANION,       /* Law 5: vehicle clause */
    WUBU_LAW_COUNT
};

typedef struct laws_result {
    int satisfied;          /* 0..5 */
    int compliant;          /* 1 when all five satisfied */
    const char *verdict;    /* static verdict label */
} laws_result;

/* flags: array of WUBU_LAW_COUNT ints, 1 = satisfied, 0 = violated. */
laws_result wubu_laws_check(const int *flags);

/* Static law text for display. */
const char *wubu_law_text(int law_index);

#ifdef __cplusplus
}
#endif

#endif /* WUBU_LAWS_H */
