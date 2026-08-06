/* better_service_cli — enforce / serve. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "better_service.h"

int main(int argc, char **argv) {
    const char *mode = (argc > 1) ? argv[1] : "serve";
    int years = (argc > 3 && strcmp(argv[2], "--years") == 0) ? atoi(argv[3]) : 25;

    service_mode m = (strcmp(mode, "enforce") == 0) ? SERVICE_ENFORCE : SERVICE_SERVE;
    service_result r;
    if (better_service_run(m, years, &r) != 0) return 1;
    printf("=== %s ===\n", r.label);
    printf("after %d years:\n", years);
    printf("  adoption   : %.1f%%\n", r.final_adoption * 100.0);
    printf("  resentment : %.2f\n", r.final_resentment);
    return 0;
}
