/* vacant_lot_cli — capture / generations / lvt. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vacant_lot.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "usage: vacant_lot capture --bought B --sold S [--years Y] | "
                "generations --price-then P --income-then I --price-now P --income-now I | "
                "lvt --value V [--rate R]\n");
        return 2;
    }
    if (strcmp(argv[1], "capture") == 0) {
        double bought = 0, sold = 0;
        int years = 20;
        for (int i = 2; i + 1 < argc; i += 2) {
            if (strcmp(argv[i], "--bought") == 0) bought = atof(argv[i + 1]);
            else if (strcmp(argv[i], "--sold") == 0) sold = atof(argv[i + 1]);
            else if (strcmp(argv[i], "--years") == 0) years = atoi(argv[i + 1]);
        }
        lot_capture c = vacant_lot_capture(bought, sold, years);
        printf("THE VACANT LOT - %.0f -> %.0f over %d years\n", bought, sold, years);
        printf("  unearned gain : $%.0f ($%.0f/yr)\n", c.unearned_gain, c.per_year);
        printf("  owner's work  : $0 - the lot did nothing\n");
        printf("  society's share: %.0f%%\n", c.society_share * 100.0);
        return 0;
    }
    if (strcmp(argv[1], "generations") == 0) {
        double pt = 0, it = 0, pn = 0, in = 0;
        for (int i = 2; i + 1 < argc; i += 2) {
            if (strcmp(argv[i], "--price-then") == 0) pt = atof(argv[i + 1]);
            else if (strcmp(argv[i], "--income-then") == 0) it = atof(argv[i + 1]);
            else if (strcmp(argv[i], "--price-now") == 0) pn = atof(argv[i + 1]);
            else if (strcmp(argv[i], "--income-now") == 0) in = atof(argv[i + 1]);
        }
        gen_bridge g = vacant_lot_generations(pt, it, pn, in);
        printf("SAME EFFORT, DIFFERENT POSITION\n");
        printf("  then: house = %.1fx income\n", g.price_income_then);
        printf("  now : house = %.1fx income\n", g.price_income_now);
        printf("  filed: %.1fx harder - not anyone's evil; that's the position.\n",
               g.times_harder);
        return 0;
    }
    fprintf(stderr, "see usage\n");
    return 2;
}
