/* status_loop_cli — run one world. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "status_loop.h"

int main(int argc, char **argv) {
    const char *mode = (argc > 1) ? argv[1] : "feedback";
    int years = (argc > 3 && strcmp(argv[2], "--years") == 0) ? atoi(argv[3]) : 30;

    status_world world;
    if (strcmp(mode, "scarcity") == 0) world = STATUS_SCARCITY;
    else if (strcmp(mode, "abundance") == 0) world = STATUS_ABUNDANCE;
    else world = STATUS_FEEDBACK;

    status_result r;
    if (status_loop_run(world, years, &r) != 0) return 1;
    printf("=== %s ===\n", status_loop_label(world));
    printf("after %d years:\n", years);
    printf("  marker adoption : %.1f%%\n", r.final_adoption * 100.0);
    printf("  status value    : %.2f %s\n", r.final_status,
           r.status_dead ? "(DEAD - superiority is boring)" : "(alive - class persists)");
    return 0;
}
