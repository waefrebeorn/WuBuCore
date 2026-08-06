/* world_model_cli — aggregate forecasts. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "world_model.h"

static int parse_doubles(const char *s, double *out, size_t max) {
    size_t n = 0;
    char copy[512];
    snprintf(copy, sizeof(copy), "%s", s); /* C11: no strdup (POSIX) */
    char *tok = strtok(copy, " ");
    while (tok && n < max) {
        out[n++] = atof(tok);
        tok = strtok(NULL, " ");
    }
    return (int)n;
}

int main(int argc, char **argv) {
    const char *human_s = NULL, *skills_s = NULL, *silicon_s = NULL;
    double hw = 0.5;
    for (int i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "--human") == 0) human_s = argv[i + 1];
        else if (strcmp(argv[i], "--skills") == 0) skills_s = argv[i + 1];
        else if (strcmp(argv[i], "--silicon") == 0) silicon_s = argv[i + 1];
        else if (strcmp(argv[i], "--human-weight") == 0) hw = atof(argv[i + 1]);
    }
    if (!human_s) {
        fprintf(stderr, "usage: world_model --human \"f1 f2 ...\" "
                        "[--skills \"s1 s2 ...\"] [--silicon \"f1 f2 ...\"]\n");
        return 2;
    }

    double human[64], skills[64], silicon[64];
    int nh = parse_doubles(human_s, human, 64);
    int ns = skills_s ? parse_doubles(skills_s, skills, 64) : 0;
    if (ns != 0 && ns != nh) {
        fprintf(stderr, "error: %d forecasts but %d skills\n", nh, ns);
        return 1;
    }

    aggregate_result ha = world_model_aggregate(human, ns ? skills : NULL,
                                                (size_t)nh);
    printf("WORLD MODEL AGGREGATION - %d human forecasts\n", nh);
    printf("  simple mean     : %.3f   (naive baseline)\n", ha.mean);
    printf("  median          : %.3f   (the robust floor)\n", ha.median);
    printf("  skill-weighted  : %.3f   (superforecaster upgrade)\n",
           ha.skill_weighted);
    printf("  recalibrated    : %.3f   (bias-corrected)\n", ha.recalibrated);

    if (silicon_s) {
        int nsi = parse_doubles(silicon_s, silicon, 64);
        aggregate_result sa = world_model_aggregate(silicon, NULL, (size_t)nsi);
        double h = world_model_hybrid(&ha, &sa, hw);
        printf("  silicon aggregate: %.3f (recalibrated)\n", sa.recalibrated);
        printf("  HYBRID (human %.0f%% + silicon %.0f%%): %.3f\n",
               hw * 100.0, (1.0 - hw) * 100.0, h);
    }
    return 0;
}
