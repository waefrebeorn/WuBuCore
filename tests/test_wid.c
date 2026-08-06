/* test_wid — verify against the known-good Python outputs. */
#include <stdio.h>
#include <string.h>

#include "wid.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)

int main(void) {
    char out[WID_MAX];

    /* Verified: WID-WB-26-0001-8 is VALID, ...-7 is INVALID */
    CK(wid_generate("WB", 26, 1, out, sizeof(out)) == 0, "generate ok");
    CK(strcmp(out, "WID-WB-26-0001-8") == 0, "generate exact");
    CK(wid_validate("WID-WB-26-0001-8") == 1, "valid WID");
    CK(wid_validate("WID-WB-26-0001-7") == 0, "forged check digit rejected");
    CK(wid_validate("WID-FF-26-0042-0") == 1, "generated value valid");

    /* Made-up WID with wrong check digit must fail */
    CK(wid_validate("WID-FF-26-0042-4") == 0, "made-up check rejected");

    wid *w = wid_parse("WID-WB-26-0001-8");
    CK(w != NULL, "parse ok");
    if (w) {
        CK(strcmp(wid_country(w), "WB") == 0, "country");
        CK(wid_year(w) == 26, "year");
        CK(wid_seq(w) == 1, "seq");
        CK(wid_check(w) == 8, "check");
        wid_free(w);
    }
    CK(wid_validate("garbage") == 0, "garbage rejected");

    printf(fails ? "%d FAILURES\n" : "WID: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
