/* mirror_score_cli — the entropy audit. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mirror_score.h"

int main(int argc, char **argv) {
    int agree = 0, disagree = 0, novel = 0, confirm = 0, comfort = 0;
    for (int i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "--agree") == 0) agree = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--disagree") == 0) disagree = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--novel") == 0) novel = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--confirm") == 0) confirm = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "--comfort") == 0) comfort = atoi(argv[i + 1]);
    }
    mirror_result r = mirror_score_compute(agree, disagree, novel,
                                           confirm, comfort);
    printf("MIRROR SCORE: %.1f/100\n", r.score);
    printf("  challenge (disagree + novel): %d\n", disagree + novel);
    printf("  reflection (agree + confirm + comfort): %d\n",
           agree + confirm + comfort);
    printf("  verdict: %s\n", r.verdict);
    return 0;
}
