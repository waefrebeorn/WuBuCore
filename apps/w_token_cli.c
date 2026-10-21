/* w_token_cli.c — the W Token ceremony machine (GOV-19).
 *
 * Usage:
 *   w_token stamp <wid> <penny-key> <weight_g>
 *   w_token verify <token-id>
 *   w_token ceremony <token-id> [gifts] [start-balance] [seat]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "w_token.h"

static w_token ledger[256];
static int ledger_n = 0;

static void print_token(const w_token *t) {
    printf("token_id  : %s\n", t->token_id);
    printf("wid       : %s\n", t->wid);
    printf("penny_key : %s\n", t->penny_key);
    printf("state     : %s\n", t->state == TOKEN_STAMPED ? "stamped" :
                              t->state == TOKEN_SHIPPED ? "shipped" : "retired");
    printf("chain     : %s\n", t->chain_hash[0] ? t->chain_hash : "(not chained)");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("w_token — the W Token ceremony machine (GOV-19)\n");
        printf("  stamp    <wid> <penny-key> <weight_g>  stamp a penny with W\n");
        printf("  verify   <token-id>                    check a token id\n");
        printf("  ceremony <token-id> [gifts] [start] [seat]\n");
        return 1;
    }

    if (strcmp(argv[1], "stamp") == 0 && argc == 5) {
        const char *wid = argv[2];
        const char *penny = argv[3];
        double weight = atof(argv[4]);
        if (w_token_penny_already_stamped(penny, ledger, ledger_n)) {
            printf("REFUSED: the penny %s already carries a W.\n", penny);
            printf("The stamp happens once. The Mint never stamps again.\n");
            return 1;
        }
        int ordinal = w_token_next_ordinal(wid, ledger, ledger_n);
        w_token t;
        if (!w_token_stamp(wid, penny, weight, ordinal, &t)) {
            printf("REFUSED: weight %.2fg is not a penny class "
                   "(3.11 copper / 2.50 zinc).\n", weight);
            return 1;
        }
        w_token_chain(&t, NULL);
        ledger[ledger_n++] = t;
        printf("STAMPED: %s\n", t.token_id);
        printf("The penny stopped being a penny. It is now a token "
               "of trust, societal credit, and humanity.\n");
        print_token(&t);
        return 0;
    }

    if (strcmp(argv[1], "verify") == 0 && argc == 3) {
        char wid[32];
        int ordinal = 0;
        if (!w_token_id_valid(argv[2], wid, 32, &ordinal)) {
            printf("INVALID: %s is not a well-formed W Token id.\n", argv[2]);
            return 1;
        }
        printf("VALID: %s — citizen %s, token %d.\n", argv[2], wid, ordinal);
        return 0;
    }

    if (strcmp(argv[1], "ceremony") == 0 && argc >= 3) {
        char wid[32];
        int ordinal = 0;
        if (!w_token_id_valid(argv[2], wid, 32, &ordinal)) {
            printf("INVALID: %s is not a well-formed W Token id.\n", argv[2]);
            return 1;
        }
        w_token t;
        memset(&t, 0, sizeof(t));
        snprintf(t.token_id, sizeof(t.token_id), "%s", argv[2]);
        snprintf(t.wid, sizeof(t.wid), "%s", wid);
        t.ordinal = ordinal;
        t.state = TOKEN_STAMPED;
        int gifts = argc >= 4 ? atoi(argv[3]) : 0;
        double start = argc >= 5 ? atof(argv[4]) : 0.0;
        int seat = argc >= 6 ? atoi(argv[5]) : 0;
        w_ceremony c;
        w_token_ceremony(&t, gifts, start, seat, &c);
        printf("=== ceremony %s ===\n", c.ceremony_id);
        printf("token     : %s\n", c.token_id);
        printf("wid       : %s\n", c.wid);
        printf("moment    : a feeling-special moment, made for this person,\n");
        printf("            so they feel special. That is its whole job.\n");
        printf("applause  : %s\n", c.applause ? "the room applauds" : "silence");
        printf("gifts     : %d\n", c.gifts_count);
        printf("start     : %.2f pennies of societal credit\n", c.start_penny_balance);
        printf("seat      : %s\n", c.seat_granted ? "granted (Ship, GOV-15)" : "not granted");
        return 0;
    }

    printf("w_token: unknown command.\n");
    return 1;
}
