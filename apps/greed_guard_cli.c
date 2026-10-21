/* greed_guard_cli.c — the Greed Index protection layer (ECO-8/GOV-20).
 *
 * Usage:
 *   greed_guard <move> <score>
 *   moves: migrate | game | buy | co-opt | legality
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "greed_guard.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("greed_guard — the Greed Index protection layer\n");
        printf("  greed_guard <move> <score>\n");
        printf("  moves: migrate | game | buy | co-opt | legality\n");
        return 1;
    }

    greed_move m;
    if (strcmp(argv[1], "migrate") == 0) m = GREED_MOVE_MIGRATE;
    else if (strcmp(argv[1], "game") == 0) m = GREED_MOVE_GAME;
    else if (strcmp(argv[1], "buy") == 0) m = GREED_MOVE_BUY;
    else if (strcmp(argv[1], "co-opt") == 0) m = GREED_MOVE_COOPT;
    else if (strcmp(argv[1], "legality") == 0) m = GREED_MOVE_LEGALITY;
    else {
        printf("unknown move: %s\n", argv[1]);
        return 1;
    }

    int score = atoi(argv[2]);
    greed_guard_result r;
    greed_guard_respond(m, score, &r);

    const char *band_name = r.band == GREED_BAND_FILED ? "FILED" :
                            r.band == GREED_BAND_WATCHED ? "WATCHED" : "FLAGGED";
    printf("GREED INDEX : %d — band %s\n", r.index_score, band_name);
    printf("ACTOR MOVE  : %s\n", greed_guard_move_name(m));
    printf("PROTECTION  : %s\n", r.response);
    return 0;
}
