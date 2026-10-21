/*
 * w_token.h — the W Token ceremony machine (GOV-19).
 *
 * The adoptable government's adoption ceremony: an old penny is
 * weighed, stamped with a W, and becomes a W Token — a token of
 * trust, societal credit, and humanity. The stamp happens once.
 * The ceremony is a documented feeling-special moment.
 */
#ifndef W_TOKEN_H
#define W_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

/* The stamp: a penny becomes a W Token exactly once. */
#define W_TOKEN_ID_MAX 64
#define W_TOKEN_WID_MAX 32
#define W_TOKEN_PENNY_KEY_MAX 48
#define W_TOKEN_CEREMONY_MAX 64
#define W_TOKEN_HASH_HEX 65 /* 64 hex chars + NUL (wubusha256) */

typedef enum token_state {
    TOKEN_STAMPED = 0,  /* the W is in the metal; the token is live */
    TOKEN_SHIPPED,      /* carried off-world / transferred */
    TOKEN_RETIRED       /* the token was willingly returned */
} token_state;

typedef struct w_token {
    char token_id[W_TOKEN_ID_MAX];          /* WT-<WID>-<ordinal> */
    char wid[W_TOKEN_WID_MAX];              /* the citizen's WID */
    char penny_key[W_TOKEN_PENNY_KEY_MAX];  /* the old penny's Coin Key */
    token_state state;
    int ordinal;                            /* N in WT-<WID>-<N> */
    char chain_hash[W_TOKEN_HASH_HEX];   /* SHA-256 hex link (PennyChain) */
} w_token;

/* Stamp a penny: it stops being a penny. Returns 1 if the stamp
 * lands (penny accepted), 0 if rejected (already stamped, or the
 * penny fails the weight standard). */
int w_token_stamp(const char *wid, const char *penny_key,
                  double weight_g, int ordinal, w_token *out);

/* A token id is well-formed: WT-<WID>-<ordinal>. */
int w_token_id_valid(const char *token_id, char *wid_out, int wid_max,
                     int *ordinal_out);

/* The one-stamp rule: a penny key may become a token only once. */
int w_token_penny_already_stamped(const char *penny_key,
                                  const w_token *ledger, int ledger_n);

/* A W Token is not re-minted; the cap never grows from the stamp.
 * Returns the ordinal that keeps the token under the cap. */
int w_token_next_ordinal(const char *wid, const w_token *ledger,
                         int ledger_n);

/* The ceremony record (the feeling-special moment). Fields are the
 * documented purpose: the moment exists so the person feels
 * special. Honesty is part of the gift. */
typedef struct w_ceremony {
    char ceremony_id[W_TOKEN_CEREMONY_MAX]; /* ceremony-<WID>-<ordinal> */
    char token_id[W_TOKEN_ID_MAX];
    char wid[W_TOKEN_WID_MAX];
    int applause;        /* the room applauds (1 = yes, always) */
    int gifts_count;     /* gifts given to the person's start */
    double start_penny_balance;  /* societal credit at the start */
    int seat_granted;    /* a seat on the Ship (GOV-15) */
} w_ceremony;

/* Generate the ceremony record. applause is always 1: the room
 * applauds by the doctrine. gifts_count >= 0. */
void w_token_ceremony(const w_token *tok, int gifts_count,
                      double start_penny_balance, int seat_granted,
                      w_ceremony *out);

/* Chain the token into the ledger: SHA-256 over the previous
 * entry's hash (hex). Returns 1 on success. */
int w_token_chain(const w_token *tok, const char prev_hash_hex[W_TOKEN_HASH_HEX]);

#ifdef __cplusplus
}
#endif

#endif /* W_TOKEN_H */
