/* test_eco_sims — status_loop, numbing, better_service, vacant_lot,
 * lifespan_loop, greed_index: the verified economics numbers. */
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "status_loop.h"
#include "numbing.h"
#include "better_service.h"
#include "vacant_lot.h"
#include "lifespan_loop.h"
#include "greed_index.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 0.01)

int main(void) {
    /* status_loop: scarcity 1.00, feedback 0.00 (verified) */
    status_result sr;
    status_loop_run(STATUS_SCARCITY, 30, &sr);
    CK(NEAR(sr.final_status, 1.00), "scarcity status 1.00");
    CK(!sr.status_dead, "scarcity class persists");
    status_loop_run(STATUS_FEEDBACK, 30, &sr);
    CK(sr.status_dead, "feedback status dead");
    CK(NEAR(sr.final_status, 0.00), "feedback status 0.00");

    /* numbing: calm 0.95, nexus 0.09 (verified) */
    numbing_result nr;
    numbing_run(NUMBING_CALM, 5, &nr);
    CK(NEAR(nr.final_sensitivity, 0.95), "calm 0.95");
    CK(!nr.numbed, "calm not numbed");
    numbing_run(NUMBING_NEXUS, 5, &nr);
    CK(NEAR(nr.final_sensitivity, 0.09), "nexus 0.09");
    CK(nr.numbed, "nexus numbed");

    /* better_service: enforce 28.5% / serve 95.5% at 25y (verified) */
    service_result es, sv;
    better_service_run(SERVICE_ENFORCE, 25, &es);
    CK(NEAR(es.final_adoption, 0.285), "enforce adoption 28.5%");
    CK(es.final_resentment > 5.0, "enforce resentment high");
    better_service_run(SERVICE_SERVE, 25, &sv);
    CK(NEAR(sv.final_adoption, 0.955), "serve adoption 95.5%");
    CK(NEAR(sv.final_resentment, 0.0), "serve resentment 0");

    /* vacant_lot: 35000->60000 = 25000 unearned */
    lot_capture lc = vacant_lot_capture(35000, 60000, 15);
    CK(NEAR(lc.unearned_gain, 25000.0), "vacant lot gain");

    /* lifespan: 32->2 gens, 73->3-4, 84->4-5 */
    lifespan_result lr = lifespan_loop_report(32.0, 25.0);
    CK(lr.generations_alive == 2, "32yr -> 2 gens");
    lr = lifespan_loop_report(73.0, 25.0);
    CK(lr.generations_alive == 3, "73yr -> 3 gens");
    lr = lifespan_loop_report(84.0, 25.0);
    CK(lr.generations_alive == 4, "84yr -> 4 gens");

    /* greed_index: 40+30+20=90 FILED; 210 FLAGGED */
    greed_result gr = greed_index_compute(40, 30, 20);
    CK(NEAR(gr.total, 90.0), "greed 90");
    gr = greed_index_compute(80, 70, 60);
    CK(NEAR(gr.total, 210.0), "greed 210");
    CK(strcmp(gr.band, GREED_BAND_FLAGGED) == 0, "flagged band");

    printf(fails ? "%d FAILURES\n" : "ECO_SIMS: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
