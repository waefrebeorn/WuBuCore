#include "penny_weight.h"

#include "wubucore.h"

reserve_audit penny_weight_audit(double copper_g, double zinc_g) {
    reserve_audit a;
    a.copper_pennies = copper_g / COPPER_G_PER_PENNY;
    a.zinc_pennies = zinc_g / ZINC_G_PER_PENNY;
    a.total_pennies = a.copper_pennies + a.zinc_pennies;
    a.face_value_dollars = a.total_pennies / 100.0;
    return a;
}

reserve_audit penny_weight_worth(double total_g, double copper_share) {
    double cs = wubu_clamp(copper_share, 0.0, 1.0);
    return penny_weight_audit(total_g * cs, total_g * (1.0 - cs));
}

redemption penny_weight_redeem(int pennies, double copper_share) {
    double cs = wubu_clamp(copper_share, 0.0, 1.0);
    redemption r;
    r.copper_pennies = pennies * cs;
    r.zinc_pennies = pennies * (1.0 - cs);
    r.copper_weight_g = r.copper_pennies * COPPER_G_PER_PENNY;
    r.zinc_weight_g = r.zinc_pennies * ZINC_G_PER_PENNY;
    r.total_weight_kg = (r.copper_weight_g + r.zinc_weight_g) / 1000.0;
    return r;
}

shipping penny_weight_shipping(int pennies, int per_envelope) {
    shipping s;
    s.full_envelopes = pennies / per_envelope;
    s.remainder = pennies % per_envelope;
    s.envelopes_needed = s.full_envelopes + (s.remainder ? 1 : 0);
    return s;
}
