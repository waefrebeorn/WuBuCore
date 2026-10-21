/* test_w_token — the W Token ceremony machine (GOV-19). */
#include <stdio.h>
#include <string.h>

#include "w_token.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    /* The stamp: a real copper penny (3.11g) becomes a W Token. */
    w_token t;
    int ok = w_token_stamp("WID-0001", "CK-WID-0001-00001", 3.11, 1, &t);
    CK(ok, "copper penny accepted");
    CK(strcmp(t.token_id, "WT-WID-0001-00001") == 0, "token id WT-WID-ordinal");
    CK(t.state == TOKEN_STAMPED, "state stamped");
    CK(strcmp(t.penny_key, "CK-WID-0001-00001") == 0, "penny key kept");

    /* A zinc penny (2.50g) is accepted too. */
    w_token t2;
    ok = w_token_stamp("WID-0002", "CK-WID-0002-00001", 2.50, 1, &t2);
    CK(ok, "zinc penny accepted");

    /* A fake weight is refused at the door: the Mint weighs first. */
    ok = w_token_stamp("WID-0003", "CK-WID-0003-00001", 5.00, 1, &t2);
    CK(!ok, "5.00g rejected (not a penny class)");

    /* The one-stamp rule: a penny key becomes a token only once. */
    w_token ledger[2] = {t, t2};
    CK(w_token_penny_already_stamped("CK-WID-0001-00001", ledger, 2),
       "penny already stamped detected");
    CK(!w_token_penny_already_stamped("CK-WID-0009-00001", ledger, 2),
       "fresh penny passes");

    /* The next ordinal: per citizen, sequential. */
    CK(w_token_next_ordinal("WID-0001", ledger, 2) == 2,
       "next ordinal for WID-0001 is 2");
    CK(w_token_next_ordinal("WID-0002", ledger, 2) == 2,
       "next ordinal for WID-0002 is 2");
    CK(w_token_next_ordinal("WID-0099", ledger, 2) == 1,
       "new citizen starts at ordinal 1");

    /* The id check round-trips the wid and ordinal. */
    char wid[32];
    int ord = 0;
    CK(w_token_id_valid("WT-WID-0042-00007", wid, 32, &ord),
       "token id valid");
    CK(strcmp(wid, "WID-0042") == 0 && ord == 7, "wid + ordinal round-trip");
    CK(!w_token_id_valid("WT-BAD", wid, 32, &ord), "malformed id rejected");

    /* The ceremony: a feeling-special moment, filed honestly.
     * The room applauds (always), gifts count, start balance set,
     * seat granted per the societal standards. */
    w_ceremony c;
    w_token_ceremony(&t, 7, 100.00, 1, &c);
    CK(strcmp(c.ceremony_id, "ceremony-WID-0001-00001") == 0,
       "ceremony id ceremony-WID-ordinal");
    CK(c.applause == 1, "the room applauds");
    CK(c.gifts_count == 7, "seven gifts counted");
    CK(c.start_penny_balance == 100.00, "start balance 100 pennies");
    CK(c.seat_granted == 1, "seat on the Ship granted");

    /* No gifts: still a ceremony; the count is zero, never negative. */
    w_token_ceremony(&t2, -1, 0.0, 0, &c);
    CK(c.gifts_count == 0, "no gifts, count clamped to 0");
    CK(c.seat_granted == 0, "no seat without the standard");

    /* The chain: hashing is deterministic and changes with input. */
    w_token a, b;
    w_token_stamp("WID-0010", "CK-WID-0010-00001", 3.11, 1, &a);
    w_token_stamp("WID-0010", "CK-WID-0010-00001", 3.11, 1, &b);
    w_token_chain(&a, NULL);
    w_token_chain(&b, NULL);
    CK(strcmp(a.chain_hash, b.chain_hash) == 0,
       "chain deterministic");
    CK(strlen(a.chain_hash) == 64, "chain is 64 hex chars");
    b.ordinal = 2;
    snprintf(b.token_id, sizeof(b.token_id), "WT-WID-0010-00002");
    w_token_chain(&b, NULL);
    CK(strcmp(a.chain_hash, b.chain_hash) != 0,
       "chain changes with the token");

    /* Chained with the previous hash: the ledger links. */
    w_token_chain(&b, a.chain_hash);
    CK(strcmp(b.chain_hash, a.chain_hash) != 0,
       "previous hash enters the digest");

    printf(fails ? "%d FAILURES\n" : "W_TOKEN: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
