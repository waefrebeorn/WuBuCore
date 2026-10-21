/*
 * greed_guard.h — the Greed Index protection layer (ECO-8, GOV-20).
 *
 * The Index locates greed; the Guard answers it. For each of the
 * five malicious-compliance moves the Index anticipates, the Guard
 * returns the space's protection response. The space has no power
 * to punish; its protection is structural — filing, rotation,
 * append-only ledgers, and the license that converts co-optation
 * into joining.
 */
#ifndef GREED_GUARD_H
#define GREED_GUARD_H

#ifdef __cplusplus
extern "C" {
#endif

#define GREED_GUARD_RESPONSE_MAX 160

typedef enum greed_move {
    GREED_MOVE_MIGRATE = 0,  /* shift citizenship to escape filing */
    GREED_MOVE_GAME,         /* fake pennies, fake reports, fake scores */
    GREED_MOVE_BUY,          /* buy the Bureau, the scores, the license */
    GREED_MOVE_COOPT,        /* copy the license to co-opt the space */
    GREED_MOVE_LEGALITY,     /* attack the space's legal standing */
    GREED_MOVE_COUNT
} greed_move;

typedef enum greed_band {
    GREED_BAND_FILED = 0,    /* 0-99   observed, logged, watched with love */
    GREED_BAND_WATCHED,      /* 100-199 spotlight intensifies; WBI-1 monitoring */
    GREED_BAND_FLAGGED       /* 200+   ledger publishes; illegal action reported */
} greed_band;

typedef struct greed_guard_result {
    greed_band band;
    int index_score;                       /* capture + nexus + malice */
    char response[GREED_GUARD_RESPONSE_MAX]; /* the protection response */
} greed_guard_result;

/* The five moves, named. */
const char *greed_guard_move_name(greed_move m);

/* The band for a score (0-300). */
greed_band greed_guard_band(int index_score);

/* The protection response for a move at a band. */
void greed_guard_respond(greed_move m, int index_score,
                         greed_guard_result *out);

/* The ledger never forgets: every protection response is a row.
 * Returns 1 if the response is recordable. */
int greed_guard_recordable(const greed_guard_result *r);

#ifdef __cplusplus
}
#endif

#endif /* GREED_GUARD_H */
