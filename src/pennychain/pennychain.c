#include "pennychain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wubusha256.h"

/* One mint interval: the forest root, the previous root (chain),
 * the sealed hash, and the leaf hashes for verification. */
typedef struct mint {
    char root_hex[WUBU_SHA256_HEX];
    char prev_root_hex[WUBU_SHA256_HEX];
    char sealed_hex[WUBU_SHA256_HEX];
    size_t nrows;
    char **leaf_hex;  /* nrows entries */
    char **ids;       /* nrows entries */
} mint;

struct pennychain {
    mint *mints;
    size_t nmints;
    size_t cap;
};

static void hash_row(const penny_row *r, char *out) {
    /* Deterministic concatenation: all fields joined by '|'. */
    char buf[512];
    snprintf(buf, sizeof(buf), "%s|%s|%s|%s|%s|%s",
             r->penny_id, r->received_date, r->sender_wid,
             r->sender_name, r->project, r->type);
    wubu_sha256_hex((const unsigned char *)buf, strlen(buf), out);
}

static void hash_pair(const char *a, const char *b, char *out) {
    char buf[2 * WUBU_SHA256_HEX];
    snprintf(buf, sizeof(buf), "%s%s", a, b);
    wubu_sha256_hex((const unsigned char *)buf, strlen(buf), out);
}

/* Binary Merkle root over an array of leaf hashes; odd nodes promote. */
static void merkle_root(char **leaves, size_t n, char *out) {
    if (n == 0) {
        wubu_sha256_hex((const unsigned char *)"empty", 5, out);
        return;
    }
    size_t cap = n;
    char **level = (char **)malloc(sizeof(char *) * cap);
    if (!level) { out[0] = '\0'; return; }
    for (size_t i = 0; i < n; i++) {
        level[i] = (char *)malloc(WUBU_SHA256_HEX);
        strcpy(level[i], leaves[i]);
    }
    size_t count = n;
    while (count > 1) {
        size_t next = 0;
        for (size_t i = 0; i < count; i += 2) {
            char tmp[WUBU_SHA256_HEX];
            if (i + 1 < count) {
                hash_pair(level[i], level[i + 1], tmp);
            } else {
                strcpy(tmp, level[i]); /* promote odd node */
            }
            if (next < cap) {
                strcpy(level[next], tmp);
            }
            next++;
        }
        count = next;
    }
    strcpy(out, level[0]);
    for (size_t i = 0; i < n; i++) free(level[i]);
    free(level);
}

pennychain *pennychain_new(void) {
    return (pennychain *)calloc(1, sizeof(pennychain));
}

static void free_mint(mint *m) {
    for (size_t i = 0; i < m->nrows; i++) {
        free(m->leaf_hex[i]);
        free(m->ids[i]);
    }
    free(m->leaf_hex);
    free(m->ids);
}

void pennychain_free(pennychain *pc) {
    if (!pc) return;
    for (size_t i = 0; i < pc->nmints; i++) free_mint(&pc->mints[i]);
    free(pc->mints);
    free(pc);
}

int pennychain_mint(pennychain *pc, const penny_row *rows, size_t nrows) {
    if (!pc || (!rows && nrows > 0)) return -1;

    if (pc->nmints == pc->cap) {
        size_t ncap = pc->cap ? pc->cap * 2 : 4;
        mint *nm = (mint *)realloc(pc->mints, ncap * sizeof(mint));
        if (!nm) return -1;
        pc->mints = nm;
        pc->cap = ncap;
    }

    mint *m = &pc->mints[pc->nmints];
    memset(m, 0, sizeof(*m));
    m->nrows = nrows;
    if (nrows > 0) {
        m->leaf_hex = (char **)calloc(nrows, sizeof(char *));
        m->ids = (char **)calloc(nrows, sizeof(char *));
        if (!m->leaf_hex || !m->ids) return -1;
        for (size_t i = 0; i < nrows; i++) {
            m->leaf_hex[i] = (char *)malloc(WUBU_SHA256_HEX);
            m->ids[i] = (char *)malloc(32);
            if (!m->leaf_hex[i] || !m->ids[i]) return -1;
            hash_row(&rows[i], m->leaf_hex[i]);
            snprintf(m->ids[i], 32, "%s", rows[i].penny_id);
        }
    }

    char **leaves = (char **)malloc((nrows ? nrows : 1) * sizeof(char *));
    if (!leaves) return -1;
    for (size_t i = 0; i < nrows; i++) leaves[i] = m->leaf_hex[i];
    merkle_root(leaves, nrows, m->root_hex);
    free(leaves);

    if (pc->nmints > 0) {
        strcpy(m->prev_root_hex, pc->mints[pc->nmints - 1].root_hex);
    } else {
        strcpy(m->prev_root_hex, "GENESIS");
    }

    /* seal = H(root, prev_root, mint_id) */
    char seal_in[2 * WUBU_SHA256_HEX + 16];
    snprintf(seal_in, sizeof(seal_in), "%s%sM-%zu", m->root_hex,
             m->prev_root_hex, pc->nmints + 1);
    wubu_sha256_hex((const unsigned char *)seal_in, strlen(seal_in),
                    m->sealed_hex);

    pc->nmints++;
    return 0;
}

int pennychain_verify(const pennychain *pc, const char *penny_id) {
    if (!pc || !penny_id) return -1;
    if (pc->nmints == 0) return 0;
    const mint *m = &pc->mints[pc->nmints - 1];
    for (size_t i = 0; i < m->nrows; i++) {
        if (strcmp(m->ids[i], penny_id) == 0) return 1;
    }
    return 0;
}

int pennychain_latest_root(const pennychain *pc, char *out, size_t outsz) {
    if (!pc || !out || pc->nmints == 0) return -1;
    if (outsz < WUBU_SHA256_HEX) return -1;
    strcpy(out, pc->mints[pc->nmints - 1].root_hex);
    return 0;
}

size_t pennychain_mint_count(const pennychain *pc) {
    return pc ? pc->nmints : 0;
}

size_t pennychain_latest_row_count(const pennychain *pc) {
    if (!pc || pc->nmints == 0) return 0;
    return pc->mints[pc->nmints - 1].nrows;
}
