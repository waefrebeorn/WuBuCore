/* test_greed_guard — the Greed Index protection layer (ECO-8/GOV-20). */
#include <stdio.h>
#include <string.h>

#include "greed_guard.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    /* Bands: 0-99 FILED, 100-199 WATCHED, 200+ FLAGGED. */
    CK(greed_guard_band(0) == GREED_BAND_FILED, "0 = FILED");
    CK(greed_guard_band(99) == GREED_BAND_FILED, "99 = FILED");
    CK(greed_guard_band(100) == GREED_BAND_WATCHED, "100 = WATCHED");
    CK(greed_guard_band(199) == GREED_BAND_WATCHED, "199 = WATCHED");
    CK(greed_guard_band(200) == GREED_BAND_FLAGGED, "200 = FLAGGED");
    CK(greed_guard_band(300) == GREED_BAND_FLAGGED, "300 = FLAGGED");

    /* The five moves, named. */
    CK(strcmp(greed_guard_move_name(GREED_MOVE_MIGRATE), "migrate") == 0,
       "move name migrate");
    CK(strcmp(greed_guard_move_name(GREED_MOVE_GAME), "game") == 0,
       "move name game");
    CK(strcmp(greed_guard_move_name(GREED_MOVE_BUY), "buy") == 0,
       "move name buy");
    CK(strcmp(greed_guard_move_name(GREED_MOVE_COOPT), "co-opt") == 0,
       "move name co-opt");
    CK(strcmp(greed_guard_move_name(GREED_MOVE_LEGALITY),
              "legality attack") == 0, "move name legality");

    /* Migration: the space has no borders. */
    greed_guard_result r;
    greed_guard_respond(GREED_MOVE_MIGRATE, 210, &r);
    CK(r.band == GREED_BAND_FLAGGED, "migrate at 210 = FLAGGED");
    CK(r.index_score == 210, "score kept");
    CK(strstr(r.response, "no borders") != NULL, "migration: no borders");

    /* Gaming: the trend cannot be faked. */
    greed_guard_respond(GREED_MOVE_GAME, 150, &r);
    CK(r.band == GREED_BAND_WATCHED, "game at 150 = WATCHED");
    CK(strstr(r.response, "slope") != NULL, "gaming: slope answer");

    /* Buying the Bureau: there is no one to buy. */
    greed_guard_respond(GREED_MOVE_BUY, 260, &r);
    CK(strstr(r.response, "no one to buy") != NULL, "buying: no one to buy");

    /* Co-optation: copying the license IS joining. */
    greed_guard_respond(GREED_MOVE_COOPT, 90, &r);
    CK(r.band == GREED_BAND_FILED, "co-opt at 90 = FILED");
    CK(strstr(r.response, "Copying the license is joining") != NULL,
       "co-opt: copying is joining");

    /* Legality attack: nothing to seize. */
    greed_guard_respond(GREED_MOVE_LEGALITY, 280, &r);
    CK(strstr(r.response, "nothing to seize") != NULL,
       "legality: nothing to seize");

    /* Recordability: a response is a ledger row. */
    CK(greed_guard_recordable(&r), "response recordable");
    greed_guard_result empty;
    memset(&empty, 0, sizeof(empty));
    CK(!greed_guard_recordable(&empty), "empty not recordable");

    printf(fails ? "%d FAILURES\n" : "GREED_GUARD: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
