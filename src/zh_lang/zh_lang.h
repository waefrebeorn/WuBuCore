/*
 * zh_lang.h — the Chinese readability gate (GOV-10 / RND-21).
 *
 * Measures Chinese text on the published readability features so
 * that the Language Doctrine's gate speaks both tongues (Article IV).
 * The English gate is plain_lang (Flesch-Kincaid); the Chinese gate
 * is zh_lang (Lin 2022 character-level features).
 *
 * RND-21 Step 5 filed three features (Lin 2022, IJLT):
 *   1. proportion of difficult characters
 *   2. number of different (unique) characters
 *   3. average sentence length (characters per sentence)
 *
 * GOV-10 Part 3 extends the gate with three thresholds:
 *   chars_per_sentence <= 40   (shorter = clearer)
 *   char_diversity   >= 0.25    (too repetitive = too simple to read)
 *   difficult_ratio  <= 0.15    (too many rare chars = hard to read)
 *
 * "Difficult" characters: CJK Unified Ideographs that fall outside
 * the common core of the most frequently used ~3,500 characters
 * (the GB/T 25000 basic-character set covers ~98% of daily text).
 * Characters in CJK Extension blocks (U+3400..) are always difficult.
 *
 * The common-core set is a compact bitmap over U+4E00..U+9FA5
 * (21,962 code points = 2,745 bytes), initialized from the
 * high-frequency ranges in the GB/T 25000 table.
 *
 * Sentence boundaries: the full-width period U+3002 (。) is the
 * canonical terminator. ASCII period, full-width exclamation (！),
 * and full-width question (？) also terminate.
 *
 * Self-contained: no third party. C11 only.
 */
#ifndef ZH_LANG_H
#define ZH_LANG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The space's two-standard gate (GOV-10 Part 3). A Chinese filing
 * is CLEAR when all three thresholds hold. */
#define ZH_CLEAR_SENTENCE_LEN 40.0   /* chars/sentence <= 40 */
#define ZH_CLEAR_DIVERSITY    0.25   /* unique/total >= 0.25 */
#define ZH_CLEAR_DIFFICULT    0.15   /* difficult/total <= 0.15 */

#define ZH_BASE_START 0x4E00u   /* CJK Unified Ideographs start */
#define ZH_BASE_END   0x9FA5u   /* ... and end (includes U+9FA5) */

typedef struct zh_metrics {
    size_t chars;        /* total Chinese characters (excl. punctuation) */
    size_t sentences;    /* sentences (>= 1) */
    size_t unique_chars; /* distinct characters */
    size_t difficult;    /* characters outside the common core */
    double chars_per_sentence;
    double char_diversity;
    double difficult_ratio;
    int clear;          /* 1 when all three thresholds met */
} zh_metrics;

/* Score a NUL-terminated UTF-8 document. Returns 0 ok, -1 bad args. */
int zh_lang_score(const char *text, zh_metrics *out);

/* Verdict label for a score (static). */
const char *zh_lang_verdict(const zh_metrics *m);

/* Is this code point a CJK Unified Ideograph or Extension? */
int zh_lang_is_cjk(unsigned int cp);

#ifdef __cplusplus
}
#endif

#endif /* ZH_LANG_H */
