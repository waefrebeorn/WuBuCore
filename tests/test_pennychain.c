/* test_pennychain — mint, verify, tamper detection. */
#include <stdio.h>
#include <string.h>

#include "pennychain.h"
#include "wubusha256.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    penny_row rows[3];
    memset(rows, 0, sizeof(rows));
    snprintf(rows[0].penny_id, sizeof(rows[0].penny_id), "P-0001");
    snprintf(rows[0].sender_wid, sizeof(rows[0].sender_wid), "WID-WB-26-0001-8");
    snprintf(rows[0].type, sizeof(rows[0].type), "COMPLIANCE");
    snprintf(rows[1].penny_id, sizeof(rows[1].penny_id), "P-0002");
    snprintf(rows[1].type, sizeof(rows[1].type), "COMPLIANCE");
    snprintf(rows[2].penny_id, sizeof(rows[2].penny_id), "P-0003");
    snprintf(rows[2].type, sizeof(rows[2].type), "GIFT");

    pennychain *pc = pennychain_new();
    CK(pc != NULL, "new");
    CK(pennychain_mint(pc, rows, 3) == 0, "mint 1");
    CK(pennychain_mint_count(pc) == 1, "count 1");
    CK(pennychain_verify(pc, "P-0001") == 1, "P-0001 proven");
    CK(pennychain_verify(pc, "P-0002") == 1, "P-0002 proven");
    CK(pennychain_verify(pc, "NOPE") == 0, "unknown not found");

    char root1[WUBU_SHA256_HEX], root2[WUBU_SHA256_HEX];
    CK(pennychain_latest_root(pc, root1, sizeof(root1)) == 0, "root 1");

    /* Chain-of-chains: same rows, second mint, same root, new seal */
    CK(pennychain_mint(pc, rows, 3) == 0, "mint 2");
    CK(pennychain_mint_count(pc) == 2, "count 2");
    CK(pennychain_latest_root(pc, root2, sizeof(root2)) == 0, "root 2");
    CK(strcmp(root1, root2) == 0, "same rows -> same root");

    /* Tamper detection: change one row's name -> different leaf -> root */
    penny_row tampered[3] = {rows[0], rows[1], rows[2]};
    snprintf(tampered[0].sender_name, sizeof(tampered[0].sender_name),
             "EVIL EDITOR");
    pennychain *pc2 = pennychain_new();
    pennychain_mint(pc2, tampered, 3);
    char root3[WUBU_SHA256_HEX];
    pennychain_latest_root(pc2, root3, sizeof(root3));
    CK(strcmp(root1, root3) != 0, "tampering changes the root");

    /* SHA-256 known vector: empty string */
    char h[WUBU_SHA256_HEX];
    CK(wubu_sha256_hex((const unsigned char *)"", 0, h) == 0, "sha empty");
    CK(strcmp(h, "e3b0c44298fc1c149afbf4c8996fb924"
                 "27ae41e4649b934ca495991b7852b855") == 0,
       "sha256('') = e3b0...");

    pennychain_free(pc);
    pennychain_free(pc2);

    printf(fails ? "%d FAILURES\n" : "PENNYCHAIN: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
