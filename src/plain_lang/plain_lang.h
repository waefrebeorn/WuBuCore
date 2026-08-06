/*
 * plain_lang.h — the Language Doctrine checker (RND-20).
 * Plain-language measurement and violation flags per the researched
 * standards: Flesch Reading Ease + Flesch-Kincaid Grade Level
 * (Flesch 1948 / Kincaid 1975), GOV.UK banned-words, passive voice.
 * Opaque struct; self-contained.
 */
#ifndef PLAIN_LANG_H
#define PLAIN_LANG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The WuBu Language Standard (RND-20): a filing is CLEAR when
 * FRES >= 60 ("plain English", 13-15yo) AND grade <= 8. */
#define PL_CLEAR_FRES 60.0
#define PL_CLEAR_GRADE 8.0

typedef struct plain_metrics {
    size_t words;
    size_t sentences;
    size_t syllables;
    double fres;        /* Flesch Reading Ease, 0-100+ */
    double grade;       /* Flesch-Kincaid grade level */
    int clear;          /* 1 when both thresholds met */
} plain_metrics;

typedef struct plain_flag {
    const char *kind;   /* "BANNED WORD" | "PASSIVE" | "LONG SENTENCE" */
    const char *detail; /* the word / the sentence excerpt */
    size_t word_index;  /* sentence index for LONG SENTENCE, else 0 */
} plain_flag;

/* Score a NUL-terminated UTF-8/ASCII document. Returns 0 ok, -1 bad args. */
int plain_lang_score(const char *text, plain_metrics *out);

/* Flag violations; *n_out receives the count (caller allocates up to
 * max_flags; pass NULL to just count). Returns 0 ok. */
int plain_lang_flags(const char *text, plain_flag *flags, size_t max_flags,
                     size_t *n_out);

/* Is the word on the banned list? (GOV.UK words-to-avoid, filed RND-20) */
int plain_lang_banned(const char *word);

/* Recommended replacement for a banned word (static string, may be NULL). */
const char *plain_lang_replacement(const char *word);

/* Verdict label for a score (static). */
const char *plain_lang_verdict(const plain_metrics *m);

#ifdef __cplusplus
}
#endif

#endif /* PLAIN_LANG_H */
