/*
 * pennychain.h — the PennyChain: keyless, penny-minted hash chain
 * (RND-3/RND-4). SHA-256 Merkle forest, minted at intervals, with
 * chain-of-chains sealing. Opaque struct; self-contained.
 */
#ifndef PENNYCHAIN_H
#define PENNYCHAIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* One ledger row (a penny or an official record). */
typedef struct penny_row {
    char penny_id[32];
    char received_date[32];
    char sender_wid[32];
    char sender_name[64];
    char project[64];
    char type[16]; /* COMPLIANCE | GIFT | CURIOSITY */
} penny_row;

/* The chain. Layout private. */
typedef struct pennychain pennychain;

pennychain *pennychain_new(void);
void pennychain_free(pennychain *pc);

/* Mint an interval: hash all rows into a Merkle forest, seal the
 * root into the chain-of-chains. Returns 0 ok, -1 on error. */
int pennychain_mint(pennychain *pc, const penny_row *rows, size_t nrows);

/* Prove a penny is a leaf of the current forest. Returns 1 proven,
 * 0 not found, -1 error. */
int pennychain_verify(const pennychain *pc, const char *penny_id);

/* Latest sealed root (hex). Returns 0 ok, -1 if never minted. */
int pennychain_latest_root(const pennychain *pc, char *out, size_t outsz);

/* Number of mints so far. */
size_t pennychain_mint_count(const pennychain *pc);

/* Number of rows in the latest mint. */
size_t pennychain_latest_row_count(const pennychain *pc);

#ifdef __cplusplus
}
#endif

#endif /* PENNYCHAIN_H */
