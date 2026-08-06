/*
 * penny_weight.h — the Penny Standard audit (RND-6): the reserve
 * is verified by MASS, not by count. Class-counted weighing.
 */
#ifndef PENNY_WEIGHT_H
#define PENNY_WEIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#define COPPER_G_PER_PENNY 3.11 /* pre-1982 copper class */
#define ZINC_G_PER_PENNY 2.50   /* Oct 1982+ zinc class */

typedef struct reserve_audit {
    double copper_pennies;
    double zinc_pennies;
    double total_pennies;
    double face_value_dollars;
} reserve_audit;

/* Audit the reserve: weigh each class (grams), count by mass. */
reserve_audit penny_weight_audit(double copper_g, double zinc_g);

/* A mixed pile: total grams + fraction that is copper (0..1). */
reserve_audit penny_weight_worth(double total_g, double copper_share);

typedef struct redemption {
    double copper_pennies;
    double zinc_pennies;
    double copper_weight_g;
    double zinc_weight_g;
    double total_weight_kg;
} redemption;

/* What a redemption ships, by weight (copper_share 0..1). */
redemption penny_weight_redeem(int pennies, double copper_share);

/* Envelope math for shipping. */
typedef struct shipping {
    int full_envelopes;
    int remainder;
    int envelopes_needed;
} shipping;

shipping penny_weight_shipping(int pennies, int per_envelope);

#ifdef __cplusplus
}
#endif

#endif /* PENNY_WEIGHT_H */
