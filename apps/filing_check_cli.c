/* filing_check_cli — the audit as a tool (GOV-11). */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filing_check.h"

int main(int argc, char **argv) {
    /* usage: filing_check collisions "NUM|TITLE" ...
     *        filing_check dangling "REF" ... -- "HAVE" ...
     *        filing_check amended FILE SUPERSEDING */
    if (argc >= 3 && strcmp(argv[1], "collisions") == 0) {
        int n = argc - 2;
        fc_number *f = (fc_number *)malloc(n * sizeof(fc_number));
        if (!f) return 1;
        for (int i = 0; i < n; i++) {
            /* argv strings are writable and outlive the loop; we
             * point into them and split on the '|' in place. */
            f[i].number = argv[i + 2];
            char *bar = strchr(argv[i + 2], '|');
            if (bar) { *bar = '\0'; f[i].title = bar + 1; }
            else f[i].title = "";
        }
        size_t collisions = 0;
        if (fc_collisions(f, (size_t)n, &collisions) != 0) {
            fprintf(stderr, "bad filings\n");
            return 2;
        }
        printf("NUMBERING AUDIT: %d filings, %zu collision(s)\n",
               n, collisions);
        printf(collisions ? "  DEFECT: duplicate numbers filed\n"
                          : "  CLEAR: no duplicate numbers\n");
        free(f);
        return 0;
    }
    if (argc >= 4 && strcmp(argv[1], "dangling") == 0) {
        /* split at the -- */
        int split = -1;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--") == 0) { split = i; break; }
        }
        if (split < 3) {
            fprintf(stderr, "usage: filing_check dangling REF... -- HAVE...\n");
            return 2;
        }
        int nrefs = split - 2;
        int nhave = argc - split - 1;
        const char **refs = (const char **)malloc(nrefs * sizeof(char *));
        const char **have = (const char **)malloc(nhave * sizeof(char *));
        if (!refs || !have) return 1;
        for (int i = 0; i < nrefs; i++) refs[i] = argv[2 + i];
        for (int i = 0; i < nhave; i++) have[i] = argv[split + 1 + i];
        size_t dangling = 0;
        if (fc_dangling(refs, (size_t)nrefs, have, (size_t)nhave,
                        &dangling) != 0) {
            return 2;
        }
        printf("DANGLING AUDIT: %d refs, %zu dangling\n", nrefs, dangling);
        printf(dangling ? "  DEFECT: referenced-but-never-filed\n"
                        : "  CLEAR: every reference has a filing\n");
        free(refs);
        free(have);
        return 0;
    }
    if (argc == 4 && strcmp(argv[1], "amended") == 0) {
        FILE *fp = fopen(argv[2], "rb");
        if (!fp) { fprintf(stderr, "cannot read %s\n", argv[2]); return 2; }
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *buf = (char *)malloc(sz + 1);
        if (!buf) return 1;
        size_t got = fread(buf, 1, sz, fp);
        buf[got] = 0;
        fclose(fp);
        int r = fc_amended(buf, argv[3]);
        printf("AMENDMENT POINTER: %s -> %s : %s\n", argv[2], argv[3],
               r == 1 ? "POINTER PRESENT" :
               r == 0 ? "MISSING (unamended amendment)" : "ERROR");
        free(buf);
        return r == 1 ? 0 : (r == 0 ? 1 : 2);
    }
    fprintf(stderr,
            "usage: filing_check collisions \"NUM|TITLE\" ...\n"
            "       filing_check dangling REF... -- HAVE...\n"
            "       filing_check amended FILE SUPERSEDING\n");
    return 2;
}
