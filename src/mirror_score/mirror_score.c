#include "mirror_score.h"

#include "wubucore.h"

mirror_result mirror_score_compute(int agree, int disagree, int novel,
                                   int confirm, int comfort) {
    mirror_result r;
    r.score = 0.0;
    r.verdict = "no data - cannot audit";

    int total = agree + disagree + novel + confirm + comfort;
    if (total <= 0) return r;

    int reflection = agree + confirm + comfort;

    double share = (double)reflection / (double)total;
    double comfort_penalty = 0.5 * ((double)comfort / (double)total);
    r.score = wubu_clamp(share * 100.0 * (1.0 + comfort_penalty), 0.0, 100.0);

    if (r.score < 20.0) {
        r.verdict = "TOOL - healthy entropy, challenges and surprises at a "
                    "good rate";
    } else if (r.score < 40.0) {
        r.verdict = "MOSTLY TOOL - some reflection, acceptable";
    } else if (r.score < 60.0) {
        r.verdict = "LEANING MIRROR - reflects more than it challenges; watch";
    } else if (r.score < 80.0) {
        r.verdict = "MIRROR - mostly confirms; recalibrate per GOV-3 Art. VI";
    } else {
        r.verdict = "PURE MIRROR - echo chamber; prohibited; retire or rebuild";
    }
    return r;
}
