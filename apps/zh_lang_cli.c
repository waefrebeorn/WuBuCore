/* zh_lang_cli — the Chinese readability gate (GOV-10 / RND-21).
 * Reads a file (or stdin with -), scores it on the three Chinese
 * readability features, reports CLEAR or REWRITE. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zh_lang.h"

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
    const char *path = (argc >= 2) ? argv[1] : "-";
    size_t len = 0;
    char *text = read_all(path, &len);
    if (!text) {
        fprintf(stderr, "cannot read %s\n", path);
        return 1;
    }

    zh_metrics m;
    if (zh_lang_score(text, &m) != 0) {
        fprintf(stderr, "scoring error\n");
        free(text);
        return 2;
    }

    printf("=== CHINESE READABILITY GATE (GOV-10 / RND-21) ===\n");
    if (m.chars == 0) {
        printf("chars             : 0\n");
        printf("verdict           : %s\n", zh_lang_verdict(&m));
        free(text);
        return 1;
    }
    printf("chars             : %zu\n", m.chars);
    printf("sentences         : %zu\n", m.sentences);
    printf("unique chars      : %zu\n", m.unique_chars);
    printf("difficult chars   : %zu\n", m.difficult);
    printf("chars/sentence    : %.1f  (clear <= %.0f)\n",
           m.chars_per_sentence, ZH_CLEAR_SENTENCE_LEN);
    printf("char diversity    : %.3f  (clear >= %.2f)\n",
           m.char_diversity, ZH_CLEAR_DIVERSITY);
    printf("difficult ratio   : %.3f  (clear <= %.2f)\n",
           m.difficult_ratio, ZH_CLEAR_DIFFICULT);
    printf("verdict           : %s\n", zh_lang_verdict(&m));

    free(text);
    return m.clear ? 0 : 1;
}
