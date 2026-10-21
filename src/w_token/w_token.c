#include "w_token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wubucore.h"
#include "wubusha256.h"

/* The penny standard, from the Mint (GOV-14): a penny that fails
 * the weight standard does not enter the ceremony. */
#define COPPER_G_PER_PENNY 3.11 /* pre-1982 copper class */
#define ZINC_G_PER_PENNY 2.50   /* Oct 1982+ zinc class */

static int weight_ok(double g) {
    /* Accept the two classes; a penny must be one of them. */
    return (g > COPPER_G_PER_PENNY - 0.01 && g < COPPER_G_PER_PENNY + 0.01) ||
           (g > ZINC_G_PER_PENNY - 0.01 && g < ZINC_G_PER_PENNY + 0.01);
}

int w_token_stamp(const char *wid, const char *penny_key,
                  double weight_g, int ordinal, w_token *out) {
    if (!wid || !penny_key || !out) return 0;
    if (!weight_ok(weight_g)) return 0;  /* the Mint refuses */
    if (ordinal < 1) return 0;
    if ((int)strlen(wid) >= W_TOKEN_WID_MAX) return 0;
    if ((int)strlen(penny_key) >= W_TOKEN_PENNY_KEY_MAX) return 0;

    snprintf(out->token_id, sizeof(out->token_id), "WT-%s-%05d", wid, ordinal);
    snprintf(out->wid, sizeof(out->wid), "%s", wid);
    snprintf(out->penny_key, sizeof(out->penny_key), "%s", penny_key);
    out->state = TOKEN_STAMPED;
    out->ordinal = ordinal;
    memset(out->chain_hash, 0, W_TOKEN_HASH_HEX);
    return 1;
}

int w_token_id_valid(const char *token_id, char *wid_out, int wid_max,
                     int *ordinal_out) {
    if (!token_id || strncmp(token_id, "WT-", 3) != 0) return 0;
    const char *dash = strrchr(token_id, '-');
    if (!dash) return 0;
    long wid_len = dash - (token_id + 3);
    if (wid_len <= 0 || wid_len >= wid_max) return 0;  /* WT-BAD, WT-X-1 */
    if (wid_out) {
        memcpy(wid_out, token_id + 3, (size_t)wid_len);
        wid_out[wid_len] = '\0';
    }
    if (ordinal_out) *ordinal_out = atoi(dash + 1);
    return 1;
}

int w_token_penny_already_stamped(const char *penny_key,
                                  const w_token *ledger, int ledger_n) {
    for (int i = 0; i < ledger_n; i++) {
        if (strcmp(ledger[i].penny_key, penny_key) == 0) return 1;
    }
    return 0;
}

int w_token_next_ordinal(const char *wid, const w_token *ledger,
                         int ledger_n) {
    int max_ordinal = 0;
    for (int i = 0; i < ledger_n; i++) {
        if (strcmp(ledger[i].wid, wid) == 0 && ledger[i].ordinal > max_ordinal) {
            max_ordinal = ledger[i].ordinal;
        }
    }
    return max_ordinal + 1;
}

void w_token_ceremony(const w_token *tok, int gifts_count,
                      double start_penny_balance, int seat_granted,
                      w_ceremony *out) {
    if (!tok || !out) return;
    snprintf(out->ceremony_id, sizeof(out->ceremony_id),
             "ceremony-%s-%05d", tok->wid, tok->ordinal);
    snprintf(out->token_id, sizeof(out->token_id), "%s", tok->token_id);
    snprintf(out->wid, sizeof(out->wid), "%s", tok->wid);
    out->applause = 1; /* the room applauds: the doctrine says so */
    out->gifts_count = gifts_count > 0 ? gifts_count : 0;
    out->start_penny_balance = start_penny_balance > 0.0 ? start_penny_balance : 0.0;
    out->seat_granted = seat_granted ? 1 : 0;
}

int w_token_chain(const w_token *tok, const char prev_hash_hex[W_TOKEN_HASH_HEX]) {
    if (!tok) return 0;
    /* Chain the token's fields; the previous hash enters the digest.
     * The single-shot hex API keeps the chain public and auditable. */
    char buf[512];
    size_t off;
    if (prev_hash_hex && prev_hash_hex[0]) {
        off = (size_t)snprintf(buf, sizeof(buf), "%s|%s|%s",
                               tok->token_id, tok->penny_key, prev_hash_hex);
    } else {
        off = (size_t)snprintf(buf, sizeof(buf), "%s|%s",
                               tok->token_id, tok->penny_key);
    }
    if (off >= sizeof(buf)) return 0;
    wubu_sha256_hex((const unsigned char *)buf, off, (char *)tok->chain_hash);
    return 1;
}
