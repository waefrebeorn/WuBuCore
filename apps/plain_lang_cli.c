/* plain_lang_cli — the Language Doctrine checker.
 * Reads a file (or stdin with -), scores it, lists violations. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plain_lang.h"

static char *read_all(const char *path, size_t *len) {
    FILE *fp = strcmp(path, "-") == 0 ? stdin : fopen(path, "rb");
    if (!fp) return NULL;
    size_t cap = 4096, n = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) { if (fp != stdin) fclose(fp); return NULL; }
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (n + 1 >= cap) {
            cap *= 2;
            char *nb = (char *)realloc(buf, cap);
            if (!nb) { free(buf); if (fp != stdin) fclose(fp); return NULL; }
            buf = nb;
        }
        buf[n++] = (char)c;
    }
    if (fp != stdin) fclose(fp);
    buf[n] = '\0';
    *len = n;
    return buf;
}

int main(int argc, char **argv) {
    /* --ste: run the STE gate (RND-22) instead of the FRES gate */
    int ste_mode = (argc >= 2 && strcmp(argv[1], "--ste") == 0);
    const char *path = (argc >= 2) ? (ste_mode ? argv[2] : argv[1]) : "-";
    size_t len = 0;
    char *text = read_all(path, &len);
    if (!text) {
        fprintf(stderr, "cannot read %s\n", path);
        return 1;
    }

    if (ste_mode) {
        ste_metrics s;
        plain_lang_ste(text, &s);
        printf("=== STE GATE (RND-22: ASD-STE100) ===\n");
        printf("sentences      : %zu\n", s.sentences);
        printf("long sentences : %zu  (> %d words; Rule: 20 procedural / 25 descriptive)\n",
               s.long_sentences, PL_STE_SENTENCE_CAP);
        printf("passives       : %zu  (Rule 3.6: use the active voice)\n",
               s.passives);
        printf("and/or         : %zu  (pick one: X, or Y, or both)\n",
               s.and_or);
        printf("abbreviations  : %zu  (no i.e. / e.g. / etc. in prose)\n",
               s.abbrevs);
        printf("present perfect: %zu  (use the simple past)\n",
               s.perfects);
        printf("noun clusters  : %zu  (4+ word noun runs)\n",
               s.noun_clusters);
        printf("verdict        : %s\n",
               s.ste_clear ? "STE CLEAR - disciplined technical English (RND-22)"
                           : "STE REWRITE - violations filed (RND-22)");
        free(text);
        return s.ste_clear ? 0 : 1;
    }

    plain_metrics m;
    plain_lang_score(text, &m);

    size_t nflags = 0;
    plain_lang_flags(text, NULL, 0, &nflags);
    plain_flag *flags = (plain_flag *)calloc(nflags ? nflags : 1,
                                             sizeof(plain_flag));
    size_t n = 0;
    if (flags) plain_lang_flags(text, flags, nflags, &n);

    printf("=== LANGUAGE DOCTRINE CHECK (RND-20) ===\n");
    printf("words     : %zu\n", m.words);
    printf("sentences : %zu\n", m.sentences);
    printf("syllables : %zu\n", m.syllables);
    printf("Flesch Reading Ease : %.1f  (clear >= 60)\n", m.fres);
    printf("Flesch-Kincaid grade: %.1f  (clear <= 8)\n", m.grade);
    printf("verdict   : %s\n", plain_lang_verdict(&m));
    printf("violations: %zu\n", n);
    for (size_t i = 0; i < n && i < nflags; i++) {
        const char *rep = NULL;
        if (strcmp(flags[i].kind, "BANNED WORD") == 0) {
            rep = plain_lang_replacement(flags[i].detail);
            printf("  - %s: '%s'%s%s\n", flags[i].kind, flags[i].detail,
                   rep ? " -> use '" : "", rep ? rep : "");
        } else {
            printf("  - %s: %s\n", flags[i].kind, flags[i].detail);
        }
    }

    int rc = (m.clear && n == 0) ? 0 : 1;
    free(flags);
    free(text);
    return rc;
}
