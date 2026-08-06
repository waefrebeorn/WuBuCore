/*
 * vacant_lot.h — the unearned capture (ECO-4): the vacant-lot
 * sign, the boomer bridge, and the Land Value Tax, in neutral math.
 */
#ifndef VACANT_LOT_H
#define VACANT_LOT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lot_capture {
    double unearned_gain;
    double per_year;
    double society_share; /* 1.0 = 100% social */
} lot_capture;

/* What the land made while everyone worked. */
lot_capture vacant_lot_capture(double bought, double sold, int years);

typedef struct gen_bridge {
    double price_income_then;
    double price_income_now;
    double times_harder;
} gen_bridge;

/* Same effort, different position. */
gen_bridge vacant_lot_generations(double price_then, double income_then,
                                  double price_now, double income_now);

typedef struct lvt_result {
    double annual_tax;
} lvt_result;

/* Land Value Tax on unimproved value (rate 0..1). */
lvt_result vacant_lot_lvt(double value, double rate);

#ifdef __cplusplus
}
#endif

#endif /* VACANT_LOT_H */
