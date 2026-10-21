#include "greed_guard.h"

#include <stdio.h>
#include <string.h>

const char *greed_guard_move_name(greed_move m) {
    switch (m) {
    case GREED_MOVE_MIGRATE:  return "migrate";
    case GREED_MOVE_GAME:     return "game";
    case GREED_MOVE_BUY:      return "buy";
    case GREED_MOVE_COOPT:    return "co-opt";
    case GREED_MOVE_LEGALITY: return "legality attack";
    default:                  return "unknown";
    }
}

greed_band greed_guard_band(int index_score) {
    if (index_score < 0) return GREED_BAND_FILED;
    if (index_score < 100) return GREED_BAND_FILED;
    if (index_score < 200) return GREED_BAND_WATCHED;
    return GREED_BAND_FLAGGED;
}

void greed_guard_respond(greed_move m, int index_score,
                         greed_guard_result *out) {
    if (!out) return;
    out->index_score = index_score < 0 ? 0 : (index_score > 300 ? 300 : index_score);
    out->band = greed_guard_band(out->index_score);

    switch (m) {
    case GREED_MOVE_MIGRATE:
        /* Migration changes nothing: the space has no borders;
         * citizenship is wherever the penny is. */
        snprintf(out->response, sizeof(out->response),
                 "Migration changes nothing. The space has no borders. "
                 "Citizenship is wherever the penny is; the ledger "
                 "follows the mind.");
        break;
    case GREED_MOVE_GAME:
        /* Fake pennies fail the weight; fake scores fail the trend. */
        snprintf(out->response, sizeof(out->response),
                 "Fake pennies fail the Mint's weight. Fake scores fail "
                 "the trend (Compass). One act cannot fake a slope; a "
                 "keyless chain cannot be bought.");
        break;
    case GREED_MOVE_BUY:
        /* There is no one to buy: the Bureau watches the Bureau. */
        snprintf(out->response, sizeof(out->response),
                 "There is no one to buy. The Bureau watches the "
                 "Bureau; the lowest clerk files against the highest "
                 "authority. The ledger is append-only.");
        break;
    case GREED_MOVE_COOPT:
        /* Copying the license IS joining; co-optation is conversion. */
        snprintf(out->response, sizeof(out->response),
                 "Copying the license is joining. The license "
                 "replacement strategy means co-optation converts: a "
                 "government that becomes the space is filed.");
        break;
    case GREED_MOVE_LEGALITY:
        /* Nothing to seize, arrest, or disperse. */
        snprintf(out->response, sizeof(out->response),
                 "The space is powerless, landless, voluntary, and "
                 "mind-only. There is nothing to seize, nothing to "
                 "arrest, nothing to disperse. The spotlight remains.");
        break;
    default:
        snprintf(out->response, sizeof(out->response),
                 "The Index observes and files.");
        break;
    }
}

int greed_guard_recordable(const greed_guard_result *r) {
    return r != NULL && r->response[0] != '\0';
}
