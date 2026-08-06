/* penny_weight_cli — audit / worth / redeem / shipping. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "penny_weight.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "usage: penny_weight audit --copper G --zinc G | "
                "worth --total G --copper-share F | "
                "redeem --pennies N [--copper-share F] | "
                "shipping --pennies N --per N\n");
        return 2;
    }
    if (strcmp(argv[1], "audit") == 0 && argc == 6) {
        double cg = atof(argv[3]), zg = atof(argv[5]);
        reserve_audit a = penny_weight_audit(cg, zg);
        printf("RESERVE AUDIT - class-counted weighing\n");
        printf("  copper %.0fg / 3.11 = %.0f pennies\n", cg, a.copper_pennies);
        printf("  zinc   %.0fg / 2.50 = %.0f pennies\n", zg, a.zinc_pennies);
        printf("  RESERVE: %.0f pennies = $%.2f\n",
               a.total_pennies, a.face_value_dollars);
        return 0;
    }
    if (strcmp(argv[1], "redeem") == 0 && argc >= 4) {
        int pennies = atoi(argv[3]);
        double share = (argc == 6 && strcmp(argv[4], "--copper-share") == 0)
                           ? atof(argv[5]) : 0.0;
        redemption r = penny_weight_redeem(pennies, share);
        printf("REDEMPTION - %d pennies (copper share %.0f%%)\n",
               pennies, share * 100);
        printf("  ships: %.0f copper (%.0fg) + %.0f zinc (%.0fg)\n",
               r.copper_pennies, r.copper_weight_g,
               r.zinc_pennies, r.zinc_weight_g);
        printf("  total weight: %.3f kg\n", r.total_weight_kg);
        return 0;
    }
    fprintf(stderr, "see usage\n");
    return 2;
}
