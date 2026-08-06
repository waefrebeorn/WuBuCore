#include "plain_lang.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Syllable counting: the standard heuristic — vowel groups, with the  */
/* classic -e silent endings. Approximate but consistent (Flesch used  */
/* the same family of heuristics).                                     */
/* ------------------------------------------------------------------ */

static int count_syllables_word(const char *w, size_t len) {
    if (len == 0) return 0;
    int n = 0;
    int prev_vowel = 0;
    for (size_t i = 0; i < len; i++) {
        char c = (char)tolower((unsigned char)w[i]);
        int vowel = (c == 'a' || c == 'e' || c == 'i' || c == 'o' ||
                     c == 'u' || c == 'y');
        if (vowel && !prev_vowel) n++;
        prev_vowel = vowel;
    }
    /* silent final e */
    if (len >= 2) {
        char last = (char)tolower((unsigned char)w[len - 1]);
        char prev = (char)tolower((unsigned char)w[len - 2]);
        if (last == 'e' && prev != 'e' && prev != 'a' && prev != 'i' &&
            prev != 'o' && prev != 'u') {
            if (n > 1) n--;
        }
    }
    /* every word needs at least one syllable */
    if (n < 1) n = 1;
    return n;
}

/* ------------------------------------------------------------------ */
/* Banned words (filed RND-20, from GOV.UK words-to-avoid):            */
/* ------------------------------------------------------------------ */

typedef struct banned {
    const char *word;
    const char *replacement;
} banned;

static const banned BANNED[] = {
    {"incentivise", "encourage"},
    {"incentivize", "encourage"},
    {"utilise", "use"},
    {"utilize", "use"},
    {"initiate", "start"},
    {"commence", "start"},
    {"leverage", "use"},
    {"liaise", "work with"},
    {"robust", "well thought out"},
    {"streamline", "simplify"},
    {"facilitate", "help"},
    {"endeavour", "try"},
    {"endeavor", "try"},
    {"ascertain", "find out"},
    {"approximately", "about"},
    {"subsequent", "later"},
    {"prior", "before"},
    {"terminate", "end"},
    {"purchase", "buy"},
    {"obtain", "get"},
    {"assist", "help"},
    {"additional", "more"},
    {"aforementioned", "this"},
    {"notwithstanding", "even so"},
    {"heretofore", "until now"},
    {"utilization", "use"},
};

#define NBANNED (sizeof(BANNED) / sizeof(BANNED[0]))

/* Find the static table entry for a word; NULL if not banned. The
 * returned pointers are STATIC (never freed) — safe to hold. */
static const banned *banned_find(const char *word) {
    if (!word) return NULL;
    for (size_t i = 0; i < NBANNED; i++) {
        if (strcmp(BANNED[i].word, word) == 0) return &BANNED[i];
    }
    return NULL;
}

int plain_lang_banned(const char *word) {
    return banned_find(word) != NULL;
}

const char *plain_lang_replacement(const char *word) {
    const banned *b = banned_find(word);
    return b ? b->replacement : NULL;
}

/* ------------------------------------------------------------------ */
/* Tokenizer: words + sentence ends. Punctuation splits sentences.    */
/* ------------------------------------------------------------------ */

typedef struct scan {
    size_t words;
    size_t sentences;
    size_t syllables;
    char **word_list;      /* lowercase copies, for flagging */
    size_t word_cap;
    size_t *sentence_len;  /* words per sentence, for long flags */
    size_t sent_cap;
    size_t cur_sent_words;
} scan;

static void scan_init(scan *s) {
    memset(s, 0, sizeof(*s));
    s->sentences = 0; /* counted by terminator; +1 for trailing words */
}

static void scan_word(scan *s, const char *start, size_t len) {
    if (len == 0) return;
    s->words++;
    s->syllables += (size_t)count_syllables_word(start, len);
    s->cur_sent_words++;

    if (s->word_list == NULL) {
        s->word_cap = 64;
        s->word_list = (char **)calloc(s->word_cap, sizeof(char *));
    } else if (s->words > s->word_cap) {
        size_t nc = s->word_cap * 2;
        char **nl = (char **)realloc(s->word_list, nc * sizeof(char *));
        if (!nl) return;
        s->word_list = nl;
        s->word_cap = nc;
    }
    char *copy = (char *)malloc(len + 1);
    if (!copy) return;
    for (size_t i = 0; i < len; i++) {
        copy[i] = (char)tolower((unsigned char)start[i]);
    }
    copy[len] = '\0';
    s->word_list[s->words - 1] = copy;
}

static void scan_sentence_end(scan *s) {
    if (s->sentence_len == NULL) {
        s->sent_cap = 32;
        s->sentence_len = (size_t *)calloc(s->sent_cap, sizeof(size_t));
    } else if (s->sentences >= s->sent_cap) {
        size_t nc = s->sent_cap * 2;
        size_t *nl = (size_t *)realloc(s->sentence_len, nc * sizeof(size_t));
        if (!nl) return;
        s->sentence_len = nl;
        s->sent_cap = nc;
    }
    s->sentence_len[s->sentences] = s->cur_sent_words;
    s->sentences++;
    s->cur_sent_words = 0;
}

static void scan_free(scan *s) {
    if (s->word_list) {
        for (size_t i = 0; i < s->words; i++) free(s->word_list[i]);
        free(s->word_list);
    }
    free(s->sentence_len);
}

static void scan_text(const char *text, scan *s) {
    scan_init(s);
    size_t i = 0;
    size_t n = strlen(text);
    while (i < n) {
        unsigned char c = (unsigned char)text[i];
        if (isalnum(c)) {
            /* a token: alphanumerics plus word-connectors (. @ / - &) */
            size_t start = i;
            int pure_alpha = 1;
            while (i < n) {
                unsigned char d = (unsigned char)text[i];
                if (isalnum(d)) {
                    if (!isalpha(d)) pure_alpha = 0;
                    i++;
                } else if (d == '.' || d == '@' || d == '/' || d == '-' ||
                           d == '&' || d == '_') {
                    /* a connector joins only when more alphanumerics
                     * follow: URL (waefrebeorn.org), email (@), R&D.
                     * A trailing '.' must stay a sentence end. */
                    if (i + 1 < n && isalnum((unsigned char)text[i + 1])) {
                        pure_alpha = 0;
                        i++;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
            /* only pure-alpha tokens are words: URLs, emails, numbers,
             * and hyphenated terms are not readability words */
            if (pure_alpha) {
                scan_word(s, text + start, i - start);
            }
        } else if (c == '.' || c == '!' || c == '?' || c == '|' ||
                   c == ';') {
            /* . ! ? end sentences; | splits table cells (each cell is
             * a unit, not prose); ; is a soft break */
            if (s->cur_sent_words > 0) scan_sentence_end(s);
            i++;
        } else if (c == '\n') {
            /* a newline starts a line; a list marker (-, *, digit.)
             * on that line starts a new unit */
            size_t j = i + 1;
            while (j < n && (text[j] == ' ' || text[j] == '\t')) j++;
            if (j < n && (text[j] == '-' || text[j] == '*' ||
                          text[j] == '|')) {
                if (s->cur_sent_words > 0) scan_sentence_end(s);
            }
            i++;
        } else {
            i++;
        }
    }
    if (s->cur_sent_words > 0) scan_sentence_end(s);
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

int plain_lang_score(const char *text, plain_metrics *out) {
    if (!text || !out) return -1;
    scan s;
    scan_text(text, &s);

    out->words = s.words;
    out->sentences = (s.sentences > 0) ? s.sentences : 1;
    out->syllables = s.syllables;

    if (s.words == 0) {
        out->fres = 0.0;
        out->grade = 0.0;
        out->clear = 0;
        scan_free(&s);
        return 0;
    }

    double wps = (double)s.words / (double)out->sentences;
    double spw = (double)s.syllables / (double)s.words;
    out->fres = 206.835 - 1.015 * wps - 84.6 * spw;
    out->grade = 0.39 * wps + 11.8 * spw - 15.59;
    if (out->grade < 0.0) out->grade = 0.0;
    out->clear = (out->fres >= PL_CLEAR_FRES && out->grade <= PL_CLEAR_GRADE);

    scan_free(&s);
    return 0;
}

int plain_lang_flags(const char *text, plain_flag *flags, size_t max_flags,
                     size_t *n_out) {
    if (!text || !n_out) return -1;
    *n_out = 0;
    scan s;
    scan_text(text, &s);

    /* banned words */
    for (size_t i = 0; i < s.words && (flags == NULL || *n_out < max_flags); i++) {
        const banned *b = banned_find(s.word_list[i]);
        if (b) {
            if (flags) {
                flags[*n_out].kind = "BANNED WORD";
                flags[*n_out].detail = b->word; /* static, never freed */
                flags[*n_out].word_index = i;
            }
            (*n_out)++;
        }
    }

    /* long sentences: > 25 words */
    for (size_t j = 0; j < s.sentences &&
                     (flags == NULL || *n_out < max_flags); j++) {
        if (s.sentence_len[j] > 25) {
            if (flags) {
                flags[*n_out].kind = "LONG SENTENCE";
                flags[*n_out].detail = ">25 words";
                flags[*n_out].word_index = j;
            }
            (*n_out)++;
        }
    }

    scan_free(&s);
    return 0;
}

const char *plain_lang_verdict(const plain_metrics *m) {
    if (!m) return "";
    if (m->clear) {
        return "CLEAR - plain English, no rewrite needed (RND-20)";
    }
    if (m->fres < 30.0) {
        return "VERY DIFFICULT - college-graduate level; rewrite per RND-20";
    }
    if (m->fres < 50.0) {
        return "DIFFICULT - 10th-12th grade; rewrite per RND-20";
    }
    if (m->grade > PL_CLEAR_GRADE) {
        return "GRADE TOO HIGH - shorten sentences, prefer short words";
    }
    return "NEEDS WORK - below the plain-English floor (FRES 60)";
}
