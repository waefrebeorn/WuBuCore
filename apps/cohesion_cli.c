/* cohesion_cli — score one actor or rank several (GOV-5). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cohesion.h"

static void print_one(const cohesion_actor *a) {
    cohesion_result r;
    if (cohesion_score(a, &r) != 0) {
        fprintf(stderr, "bad actor (ethos must be 0..1; given/taken >= 0)\n");
        exit(2);
    }
    printf("%s (%s)\n", a->name ? a->name : "actor",
           a->living ? "living" : "tool");
    printf("  cohesion  : %+.2f  (given %.1f - taken %.1f)\n",
           r.cohesion, a->given, a->taken);
    printf("  sapience  : %.2f  (0.6 cohesion + 0.4 ethos %.1f)\n",
           r.sapience, a->ethos);
    printf("  priority  : %.1f/100\n", r.priority);
    printf("  class     : %s\n", r.class);
    printf("  verdict   : %s\n", r.verdict);
}

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "rank") == 0 && argc >= 4) {
        /* rank NAME,GIVEN,TAKEN,ETHOS,LIVING NAME,... */
        int n = argc - 2;
        cohesion_actor *actors = (cohesion_actor *)calloc(n, sizeof(*actors));
        char (*copies)[256] = (char (*)[256])calloc(n, 256);
        if (!actors || !copies) { free(actors); free(copies); return 1; }
        for (int i = 0; i < n; i++) {
            snprintf(copies[i], 256, "%s", argv[i + 2]);
            char *fields[5] = {NULL, NULL, NULL, NULL, NULL};
            char *p = copies[i];
            for (int f = 0; f < 5; f++) {
                fields[f] = p;
                char *comma = strchr(p, ',');
                if (comma) { *comma = '\0'; p = comma + 1; }
            }
            actors[i].name = fields[0]; /* points into copies[i], persistent */
            actors[i].given = atof(fields[1]);
            actors[i].taken = atof(fields[2]);
            actors[i].ethos = atof(fields[3]);
            actors[i].living = atoi(fields[4]);
        }
        size_t *order = NULL, nout = 0;
        if (cohesion_rank(actors, (size_t)n, &order, &nout) != 0) {
            fprintf(stderr, "rank failed\n");
            free(actors);
            free(copies);
            return 1;
        }
        printf("=== PRIORITY RANK (GOV-5: sapience = cohesion + ethos) ===\n");
        for (size_t i = 0; i < nout; i++) {
            cohesion_result r;
            cohesion_score(&actors[order[i]], &r);
            printf("%2zu. %-24s priority %5.1f  %s\n", i + 1,
                   actors[order[i]].name, r.priority,
                   actors[order[i]].living ? "(living)" : "(tool)");
        }
        free(order);
        free(actors);
        free(copies);
        return 0;
    }
    if (argc == 7 && strcmp(argv[1], "score") == 0) {
        cohesion_actor a;
        memset(&a, 0, sizeof(a));
        a.name = argv[2];
        a.given = atof(argv[3]);
        a.taken = atof(argv[4]);
        a.ethos = atof(argv[5]);
        a.living = atoi(argv[6]);
        print_one(&a);
        return 0;
    }
    fprintf(stderr,
            "usage: cohesion score NAME GIVEN TAKEN ETHOS LIVING(0|1)\n"
            "       cohesion rank NAME,GIVEN,TAKEN,ETHOS,LIVING ...\n");
    return 2;
}
