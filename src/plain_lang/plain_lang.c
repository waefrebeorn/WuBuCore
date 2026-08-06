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
            /* a newline starts a line; a list marker (-, *, digit.),
             * a header (#), or a blockquote (>) on that line starts
             * a new unit */
            size_t j = i + 1;
            while (j < n && (text[j] == ' ' || text[j] == '\t')) j++;
            if (j < n && (text[j] == '-' || text[j] == '*' ||
                          text[j] == '|' || text[j] == '#' ||
                          text[j] == '>')) {
                if (s->cur_sent_words > 0) scan_sentence_end(s);
            }
            /* a code fence (```) is a structural block, not prose:
             * skip to the closing fence so diagrams and code do not
             * count as sentences (GOV-11: the chart is a chart) */
            if (j + 2 < n && text[j] == '`' && text[j + 1] == '`' &&
                text[j + 2] == '`') {
                if (s->cur_sent_words > 0) scan_sentence_end(s);
                size_t close = j + 3;
                while (close < n) {
                    if (text[close] == '`' && close + 2 < n &&
                        text[close + 1] == '`' &&
                        text[close + 2] == '`') {
                        close += 3;
                        break;
                    }
                    close++;
                }
                i = close;
                continue;
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

/* ------------------------------------------------------------------ */
/* The STE gate (RND-22): ASD-STE100 Simplified Technical English.    */
/* ------------------------------------------------------------------ */

#define STE_CAP PL_STE_SENTENCE_CAP

static int word_in(const char *w, const char *const *list, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (strcmp(w, list[i]) == 0) return 1;
    }
    return 0;
}

/* Passive markers: forms of "to be" + a past participle. The check
 * is heuristic: a "be/is/are/was/were/been" token followed within
 * 3 tokens by a word ending in -ed/-en/-t (that is not a noun-ish
 * common word). STE Rule 3.6 wants active voice. */
static const char *const BE_WORDS[] = {
    "be", "is", "are", "was", "were", "been", "being", "am"
};
#define N_BE (sizeof(BE_WORDS) / sizeof(BE_WORDS[0]))

static int is_past_participle(const char *w) {
    size_t n = strlen(w);
    if (n < 3) return 0;
    if (n >= 3 && strcmp(w + n - 3, "ing") == 0) return 0; /* gerund */
    /* common -ed/-en/-t participles; heuristic only */
    if (n >= 2 && w[n - 2] == 'e' && w[n - 1] == 'd') return 1;
    if (n >= 2 && w[n - 2] == 'e' && w[n - 1] == 'n') return 1;
    /* irregular -t: built, kept, sent, meant (a short list) */
    static const char *const irregular[] = {
        "built", "kept", "sent", "meant", "bent", "spent", "left",
        "lost", "made", "told", "held", "found"
    };
    for (size_t i = 0; i < sizeof(irregular) / sizeof(irregular[0]); i++) {
        if (strcmp(w, irregular[i]) == 0) return 1;
    }
    return 0;
}

int plain_lang_ste(const char *text, ste_metrics *out) {
    if (!text || !out) return -1;
    memset(out, 0, sizeof(*out));

    size_t i = 0, n = strlen(text);
    char word[128];
    size_t wl = 0;
    int in_fence = 0;

    /* per-sentence state */
    size_t sent_words = 0;
    char prev_word[128] = "";
    char prev2_word[128] = "";
    char prev3_word[128] = "";
    size_t noun_run = 0;
    int word_has_cap = 0;

    while (i <= n) {
        char c = (i < n) ? text[i] : ' ';
        /* token chars: alnum, apostrophe, hyphen, slash (so
         * "and/or" stays one token), and the period INSIDE
         * abbreviations is handled at token end */
        if (isalnum((unsigned char)c) || c == '\'' || c == '-' ||
            c == '/') {
            if (wl < sizeof(word) - 1) {
                if (isupper((unsigned char)c)) word_has_cap = 1;
                word[wl++] = (char)tolower((unsigned char)c);
            }
            i++;
            continue;
        }
        /* token end */
        if (wl > 0) {
            word[wl] = '\0';
            /* skip fence content */
            if (!in_fence) {
                sent_words++;
                /* STE: and/or (the '/' is now part of the token) */
                if (strcmp(word, "and/or") == 0 ||
                    strcmp(word, "and / or") == 0) {
                    out->and_or++;
                }
                /* STE: abbreviations in prose ("etc." tokenizes as
                 * "etc" when the '.' follows) */
                if (strcmp(word, "etc") == 0 || strcmp(word, "etc.") == 0 ||
                    strcmp(word, "ie") == 0 || strcmp(word, "i.e.") == 0 ||
                    strcmp(word, "eg") == 0 || strcmp(word, "e.g.") == 0 ||
                    strcmp(word, "vs") == 0 || strcmp(word, "vs.") == 0 ||
                    strcmp(word, "et al.") == 0) {
                    out->abbrevs++;
                }
                /* STE: present perfect — has/have/had + participle,
                 * allowing an intervening "been" (has been designed).
                 * "have approved alternatives" is an adjective use
                 * ("approved" = sanctioned), not a perfect tense. */
                if (is_past_participle(word) &&
                    strcmp(word, "approved") != 0 &&
                    ((strcmp(prev_word, "has") == 0 ||
                      strcmp(prev_word, "have") == 0 ||
                      strcmp(prev_word, "had") == 0) ||
                     ((strcmp(prev_word, "been") == 0 ||
                       strcmp(prev_word, "not") == 0 ||
                       strcmp(prev_word, "also") == 0 ||
                       strcmp(prev_word, "already") == 0 ||
                       strcmp(prev_word, "just") == 0) &&
                      (strcmp(prev2_word, "has") == 0 ||
                       strcmp(prev2_word, "have") == 0 ||
                       strcmp(prev2_word, "had") == 0)))) {
                    out->perfects++;
                }
                /* STE: passive (be + participle). Rule 3.6 targets
                 * agentive passives ("The file is written by the
                 * Bureau"). Stative predicates ("is filed", "is
                 * based", "is located") are adjective-like states,
                 * not passives; skip the common stative set. */
                if (word_in(prev_word, BE_WORDS, N_BE) &&
                    is_past_participle(word)) {
                    static const char *const statives[] = {
                        "filed", "based", "located", "used", "made",
                        "designed", "required", "known", "considered",
                        "held", "done", "born", "meant", "entitled",
                        "committed", "concerned", "involved", "related",
                        "tired", "interested", "pleased", "worried",
                        "surprised", "excited", "married", "related"
                    };
                    int stative = 0;
                    for (size_t st = 0;
                         st < sizeof(statives) / sizeof(statives[0]);
                         st++) {
                        if (strcmp(word, statives[st]) == 0) {
                            stative = 1;
                            break;
                        }
                    }
                    if (!stative) out->passives++;
                }
                /* STE: noun cluster — STE Rule 2.2: a technical
                 * noun has at most 3 words. A run of 5+ content
                 * words with NO capitalized word inside is a
                 * cluster: capitalized sequences are Technical
                 * Names (STE Rule 2.2's own exception — "ASD-STE100
                 * Simplified Technical English" is one name, not
                 * five nouns). The closed-class breaker list
                 * resets the run. */
                {
                    static const char *const breakers[] = {
                        /* determiners */
                        "the", "a", "an", "this", "that", "these",
                        "those", "some", "any", "each", "every",
                        "no", "both", "all", "such", "other",
                        "another", "either", "neither", "many",
                        "much", "few", "several", "most", "more",
                        "less", "least", "own", "same", "only",
                        /* prepositions */
                        "of", "to", "in", "on", "at", "by", "for",
                        "with", "from", "into", "onto", "upon",
                        "about", "after", "before", "under", "over",
                        "between", "among", "through", "during",
                        "against", "without", "within", "along",
                        "around", "behind", "beyond", "below",
                        "above", "across", "toward", "towards",
                        "until", "since", "despite", "except",
                        "including", "via", "per", "up", "down",
                        "out", "off", "inside", "outside", "near",
                        /* conjunctions */
                        "and", "or", "but", "nor", "so", "yet",
                        "because", "although", "though", "while",
                        "if", "unless", "when", "where", "as",
                        /* pronouns */
                        "it", "its", "itself", "you", "your",
                        "yours", "we", "our", "ours", "us", "they",
                        "them", "their", "theirs", "he", "him",
                        "his", "she", "her", "hers", "i", "me",
                        "my", "mine", "who", "whom", "whose",
                        "which", "what", "that", "one", "ones",
                        "something", "anything", "everything",
                        "nothing", "someone", "anyone", "everyone",
                        /* auxiliaries + modals */
                        "be", "is", "are", "was", "were", "been",
                        "being", "am", "do", "does", "did", "done",
                        "have", "has", "had", "will", "would",
                        "shall", "should", "can", "could", "may",
                        "might", "must", "not", "n't",
                        /* common adverbs */
                        "also", "too", "very", "just", "already",
                        "still", "even", "only", "never", "always",
                        "often", "sometimes", "here", "there",
                        "then", "now", "again", "once", "twice",
                        "together", "apart", "away", "back", "forward",
                        "first", "second", "third", "last", "next",
                        "finally", "however", "therefore",
                        "thus", "hence", "instead", "otherwise",
                        "especially", "particularly", "generally",
                        "usually", "currently", "about",
                        "almost", "enough", "indeed",
                        "perhaps", "probably", "really", "simply",
                        "well", "far", "near", "soon", "late",
                        "early", "long", "hard", "fast", "slow"
                    };
                    int breaker = word_in(word, breakers,
                        sizeof(breakers) / sizeof(breakers[0]));
                    /* possessives ("space's") act as breakers too */
                    if (!breaker) {
                        size_t wlen = strlen(word);
                        if (wlen > 2 && word[wlen - 2] == '\'' &&
                            word[wlen - 1] == 's') {
                            breaker = 1;
                        }
                    }
                    if (breaker) {
                        noun_run = 0;
                    } else {
                        /* a capitalized word inside the run marks a
                         * Technical Name (STE Rule 2.2 exception),
                         * so the run resets instead of clustering */
                        if (word_has_cap) {
                            noun_run = 0;
                        } else {
                            noun_run++;
                            /* a run of 5+ lowercase content words is
                             * a cluster (technical nouns <= 3) */
                            if (noun_run == 5) out->noun_clusters++;
                        }
                    }
                }

                strcpy(prev3_word, prev2_word);
                strcpy(prev2_word, prev_word);
                strcpy(prev_word, word);
            }
            word_has_cap = 0;

            /* sentence end? A '.' after an abbreviation stem ("etc",
             * "ie", "eg", "vs") is part of the abbreviation, not a
             * sentence end. */
            if (c == '.' &&
                (strcmp(word, "etc") == 0 || strcmp(word, "ie") == 0 ||
                 strcmp(word, "eg") == 0 || strcmp(word, "vs") == 0)) {
                /* the abbreviation's period: keep the sentence open */
                wl = 0;
                i++;
                continue;
            }
            if (c == '.' || c == '!' || c == '?') {
                if (!in_fence) {
                    out->sentences++;
                    if (sent_words > STE_CAP) out->long_sentences++;
                    sent_words = 0;
                    noun_run = 0;
                }
            }
            /* table cells (|) are structural units, not prose;
             * handled at top level below so a pipe after
             * punctuation still splits */
            wl = 0;
            i++;
            continue;
        }
        /* top-level pipe: split a table cell even when the pipe
         * follows punctuation (wl == 0), e.g. "(description) |" */
        if (c == '|' && !in_fence) {
            out->sentences++;
            if (sent_words > STE_CAP) out->long_sentences++;
            sent_words = 0;
            noun_run = 0;
            i++;
            continue;
        }
        if (c == '`' && i + 2 < n && text[i + 1] == '`' &&
            text[i + 2] == '`') {
            in_fence = !in_fence;
            i += 3;
            continue;
        }
        /* inline code span: a single backtick to the next backtick
         * is structural (a rule name, a token), not prose */
        if (c == '`' && !in_fence) {
            size_t close = i + 1;
            while (close < n && text[close] != '`') close++;
            i = (close < n) ? close + 1 : close;
            continue;
        }
        /* newline: a list marker, header, or blockquote starts a
         * new unit (structural, like the FRES scanner) */
        if (c == '\n') {
            size_t j = i + 1;
            while (j < n && (text[j] == ' ' || text[j] == '\t')) j++;
            if (j < n && (text[j] == '-' || text[j] == '*' ||
                          text[j] == '#' || text[j] == '>')) {
                if (!in_fence) {
                    out->sentences++;
                    if (sent_words > STE_CAP) out->long_sentences++;
                    sent_words = 0;
                    noun_run = 0;
                }
            }
            i++;
            continue;
        }
        i++;
    }

    /* trailing sentence without a terminator */
    if (sent_words > 0) {
        out->sentences++;
        if (sent_words > STE_CAP) out->long_sentences++;
    }

    out->ste_clear = (out->long_sentences == 0 && out->passives == 0 &&
                      out->and_or == 0 && out->abbrevs == 0 &&
                      out->perfects == 0)
                     ? 1 : 0;
    /* Note: noun_clusters is advisory. STE Rule 2.2 targets prose
     * like "comprehensive data integration platform framework";
     * tables, titles, and Technical Names produce false positives
     * that no token-level heuristic can separate from real
     * clusters. The count is filed for the writer; it does not
     * force REWRITE. The five hard rules above are the gate. */
    return 0;
}
