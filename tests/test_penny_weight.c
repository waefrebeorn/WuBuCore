/* test_penny_weight — the RND-6 verified audit numbers. */
#include <stdio.h>
#include <math.h>

#include "penny_weight.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 0.01)

int main(void) {
    /* Verified: 3110g copper + 25000g zinc = 11000 pennies = $110 */
    reserve_audit a = penny_weight_audit(3110.0, 25000.0);
    CK(NEAR(a.copper_pennies, 1000.0), "copper 1000");
    CK(NEAR(a.zinc_pennies, 10000.0), "zinc 10000");
    CK(NEAR(a.total_pennies, 11000.0), "total 11000");
    CK(NEAR(a.face_value_dollars, 110.0), "face $110");

    /* Copper-only audit */
    reserve_audit c = penny_weight_audit(3110.0, 0.0);
    CK(NEAR(c.total_pennies, 1000.0), "copper-only 1000");

    /* Redemption: 10000 zinc pennies = 25.000 kg */
    redemption r = penny_weight_redeem(10000, 0.0);
    CK(NEAR(r.total_weight_kg, 25.0), "10k zinc = 25 kg");

    /* Shipping: 500 at 100/envelope = 5 envelopes */
    shipping s = penny_weight_shipping(500, 100);
    CK(s.envelopes_needed == 5, "5 envelopes");

    printf(fails ? "%d FAILURES\n" : "PENNY_WEIGHT: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
