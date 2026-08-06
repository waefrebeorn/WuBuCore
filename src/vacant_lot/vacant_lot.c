#include "vacant_lot.h"

#include "wubucore.h"

lot_capture vacant_lot_capture(double bought, double sold, int years) {
    lot_capture c;
    c.unearned_gain = sold - bought;
    c.per_year = (years > 0) ? c.unearned_gain / years : 0.0;
    c.society_share = 1.0; /* 100% of a vacant lot's gain is social */
    return c;
}

gen_bridge vacant_lot_generations(double price_then, double income_then,
                                  double price_now, double income_now) {
    gen_bridge g;
    g.price_income_then = (income_then != 0.0) ? price_then / income_then : 0.0;
    g.price_income_now = (income_now != 0.0) ? price_now / income_now : 0.0;
    g.times_harder = (g.price_income_then != 0.0)
                         ? g.price_income_now / g.price_income_then
                         : 0.0;
    return g;
}

lvt_result vacant_lot_lvt(double value, double rate) {
    lvt_result r;
    r.annual_tax = value * wubu_clamp(rate, 0.0, 1.0);
    return r;
}
