#include "filing_check.h"

#include <string.h>

int fc_collisions(const fc_number *filings, size_t n, size_t *collisions) {
    if (!filings || !collisions) return -1;
    *collisions = 0;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            if (filings[i].number && filings[j].number &&
                strcmp(filings[i].number, filings[j].number) == 0) {
                (*collisions)++;
            }
        }
    }
    return 0;
}

int fc_dangling(const char **refs, size_t nrefs,
                const char **have, size_t nhave, size_t *dangling) {
    if (!refs || !have || !dangling) return -1;
    *dangling = 0;
    for (size_t i = 0; i < nrefs; i++) {
        int found = 0;
        for (size_t j = 0; j < nhave; j++) {
            if (refs[i] && have[j] && strcmp(refs[i], have[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) (*dangling)++;
    }
    return 0;
}

int fc_amended(const char *superseded_text, const char *superseding) {
    if (!superseded_text || !superseding) return -1;
    return strstr(superseded_text, superseding) ? 1 : 0;
}
