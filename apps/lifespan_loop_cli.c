/* lifespan_loop_cli — report / timeline. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lifespan_loop.h"

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "timeline") == 0) {
        timeline_point pts[6];
        if (lifespan_loop_timeline(pts, 6) != 0) return 1;
        printf("THE STACK OVER A CENTURY OF MEDICINE (gap = 25 yrs)\n");
        printf("%6s %9s %5s  friction\n", "year", "lifespan", "gens");
        for (int i = 0; i < 6; i++) {
            printf("%6d %8.0fyr %4d   %s\n", pts[i].year, pts[i].lifespan,
                   pts[i].generations_alive, pts[i].friction);
        }
        return 0;
    }
    double lifespan = (argc > 1) ? atof(argv[1]) : 73.0;
    double gap = (argc > 2) ? atof(argv[2]) : 25.0;
    lifespan_result r = lifespan_loop_report(lifespan, gap);
    printf("LIFESPAN LOOP - %.0fyr lifespan, %.0fyr gap\n", lifespan, gap);
    printf("  %.1f / %.1f = %.1f -> ~%d generations alive at once\n",
           lifespan, gap, r.raw_overlap, r.generations_alive);
    printf("  friction: %s\n", r.friction);
    return 0;
}
