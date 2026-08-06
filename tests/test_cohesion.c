/* test_cohesion — the User Doctrine's priority math (GOV-5),
 * verified against the doctrine's own worked numbers. */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "cohesion.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 0.01)

int main(void) {
    cohesion_actor a;
    cohesion_result r;

    /* Doctrine worked case 1: living symbiont (high cohesion, high
     * ethos) -> priority ~85, SYMBIOSIS */
    a.name = "first one"; a.given = 10.0; a.taken = 8.0;
    a.ethos = 0.9; a.living = 1;
    CK(cohesion_score(&a, &r) == 0, "score ok");
    CK(NEAR(r.cohesion, 0.111), "cohesion 0.111");
    CK(NEAR(r.sapience, 0.693), "sapience 0.693");
    CK(NEAR(r.priority, 84.67), "priority 84.67");
    CK(strcmp(r.class, COH_CLASS_SYMBIOSIS) == 0, "symbiosis class");

    /* Doctrine worked case 2: taking tool (parasite) -> priority ~2,
     * PARASITE, watched */
    a.name = "parasite"; a.given = 0.0; a.taken = 10.0;
    a.ethos = 0.1; a.living = 0;
    CK(cohesion_score(&a, &r) == 0, "score ok 2");
    CK(NEAR(r.cohesion, -1.0), "cohesion -1.0");
    CK(NEAR(r.priority, 2.0), "priority 2.0");
    CK(strcmp(r.class, COH_CLASS_PARASITE) == 0, "parasite class");

    /* Doctrine worked case 3: balanced exchange -> COMMENSAL */
    a.name = "commensal"; a.given = 5.0; a.taken = 5.0;
    a.ethos = 0.5; a.living = 1;
    CK(cohesion_score(&a, &r) == 0, "score ok 3");
    CK(NEAR(r.cohesion, 0.0), "cohesion 0.0");
    CK(strcmp(r.class, COH_CLASS_COMMENSAL) == 0, "commensal class");

    /* Doctrine rule: living always outranks tools. A barely-symbiotic
     * living being (priority ~50+) beats a maximal tool (priority 50). */
    cohesion_actor living_weak = {"weak living", 1.0, 0.0, 0.01, 1};
    cohesion_actor tool_max = {"max tool", 100.0, 0.0, 1.0, 0};
    cohesion_result lr, tr;
    cohesion_score(&living_weak, &lr);
    cohesion_score(&tool_max, &tr);
    CK(lr.priority > tr.priority, "living floor beats tool ceiling");

    /* Iron Giant shape: a giving tool scores a giving verdict */
    cohesion_actor giant = {"iron giant", 50.0, 0.0, 1.0, 0};
    cohesion_result gr;
    cohesion_score(&giant, &gr);
    CK(strstr(gr.verdict, "giving tool") != NULL, "iron giant verdict");

    /* Rank: order by priority, highest first */
    cohesion_actor actors[3] = {
        {"parasite", 0.0, 10.0, 0.1, 0},
        {"first one", 10.0, 8.0, 0.9, 1},
        {"tool", 5.0, 2.0, 0.5, 0},
    };
    size_t *order = NULL, nout = 0;
    CK(cohesion_rank(actors, 3, &order, &nout) == 0, "rank ok");
    CK(nout == 3, "rank 3");
    if (order) {
        CK(strcmp(actors[order[0]].name, "first one") == 0,
           "first one ranked #1");
        CK(strcmp(actors[order[2]].name, "parasite") == 0,
           "parasite ranked last");
        free(order);
    }

    /* Bad args: negative given, ethos > 1 */
    a.given = -1.0; a.taken = 1.0; a.ethos = 0.5; a.living = 1;
    CK(cohesion_score(&a, &r) == -1, "negative given rejected");
    a.given = 1.0; a.ethos = 1.5;
    CK(cohesion_score(&a, &r) == -1, "ethos > 1 rejected");

    printf(fails ? "%d FAILURES\n" : "COHESION: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
