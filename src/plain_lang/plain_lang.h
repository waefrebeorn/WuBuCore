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

/* ------------------------------------------------------------------ */
/* The STE gate (RND-22): ASD-STE100 Simplified Technical English.    */
/* FRES is the floor; STE is the discipline.                          */
/* ------------------------------------------------------------------ */

/* STE violation kinds. */
#define PL_STE_SENTENCE_LONG "STE LONG SENTENCE"
#define PL_STE_PASSIVE       "STE PASSIVE"
#define PL_STE_AND_OR        "STE AND/OR"
#define PL_STE_ABBREV        "STE ABBREV"
#define PL_STE_PERFECT       "STE PERFECT"
#define PL_STE_NOUN_CLUSTER  "STE NOUN CLUSTER"

/* The STE sentence cap: 20 words procedural / 25 descriptive.
 * The gate uses the 25 descriptive cap (documents are prose). */
#define PL_STE_SENTENCE_CAP 25

typedef struct ste_metrics {
    size_t sentences;         /* total sentences */
    size_t long_sentences;    /* > STE cap (20 procedural / 25 descriptive) */
    size_t passives;          /* passive-voice markers */
    size_t and_or;            /* "and/or" occurrences */
    size_t abbrevs;           /* i.e. / e.g. / etc. in prose */
    size_t perfects;          /* has/have/had + past participle */
    size_t noun_clusters;     /* 4+ word noun runs */
    int ste_clear;            /* 1 when no STE violations */
} ste_metrics;

/* Run the STE gate on a document. Returns 0 ok, -1 bad args. */
int plain_lang_ste(const char *text, ste_metrics *out);

#ifdef __cplusplus
}
#endif

#endif /* PLAIN_LANG_H */
