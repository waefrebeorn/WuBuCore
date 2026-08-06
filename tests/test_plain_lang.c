/* test_plain_lang — the Language Doctrine checker, verified against
 * the researched known values (Flesch 1948 / Kincaid 1975 / GOV.UK). */
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "plain_lang.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 1.0)

int main(void) {
    plain_metrics m;

    /* Verified from the research: "The cat sat on the mat." = 116 FRES */
    plain_lang_score("The cat sat on the mat.", &m);
    CK(m.words == 6, "6 words");
    CK(NEAR(m.fres, 116.0), "fres ~116 (documented example)");

    /* The platypus sentence: 24 syllables, 13 words -> 11.3 grade
     * (documented in the language doctrine's worked example) */
    plain_lang_score("The Australian platypus is seemingly a hybrid of "
                     "a mammal and reptilian creature.", &m);
    CK(m.words == 13, "platypus 13 words");
    CK(NEAR(m.grade, 11.3), "platypus grade ~11.3 (documented)");

    /* Code fences are structural, not prose (GOV-11): a diagram
     * inside a fence adds zero words and zero sentences */
    plain_lang_score("A chart:\n```\nTHE FIRST ONES the registry\n"
                     "THE STATE THE BUREAU THE HANDS\n"
                     "one long diagram line without periods\n```\n"
                     "The chart is a chart.", &m);
    CK(m.words == 7, "fence skipped: 7 words (A chart: The chart is a chart.)");
    CK(m.sentences == 2, "fence skipped: 2 sentences");

    /* GOV.UK banned word detected + replacement */
    CK(plain_lang_banned("utilise") == 1, "utilise banned");
    CK(strcmp(plain_lang_replacement("utilise"), "use") == 0, "replacement");
    CK(plain_lang_banned("use") == 0, "use is fine");
    CK(plain_lang_banned("robust") == 1, "robust banned");

    /* The STE gate (RND-22). Slop: long, passive, "and/or", "etc.",
     * present perfect, noun cluster. */
    {
        ste_metrics s;
        const char *slop =
            "The aforementioned configuration parameters should be "
            "adjusted and/or verified etc. by the user, and the system "
            "has been designed to optimize the utilization of the "
            "comprehensive data integration platform framework.";
        CK(plain_lang_ste(slop, &s) == 0, "ste ok");
        CK(s.long_sentences >= 1, "ste flags long sentence");
        CK(s.passives >= 1, "ste flags passive (should be adjusted)");
        CK(s.and_or >= 1, "ste flags and/or");
        CK(s.abbrevs >= 1, "ste flags etc.");
        CK(s.perfects >= 1, "ste flags present perfect (has been)");
        CK(s.noun_clusters >= 1, "ste flags noun cluster");
        CK(s.ste_clear == 0, "slop fails STE");
    }
    /* Disciplined STE: short, active, simple past, no abbrevs. */
    {
        ste_metrics s;
        const char *clean =
            "Adjust the temperature. The Bureau filed the report. "
            "Use one term for one thing.";
        CK(plain_lang_ste(clean, &s) == 0, "ste ok 2");
        CK(s.ste_clear == 1, "disciplined prose passes STE");
    }
    /* STE ignores code fences (the org chart stays clean). */
    {
        ste_metrics s;
        const char *chart =
            "The chart is a chart.\n```\nTHE FIRST ONES the registry "
            "and the state and the bureau all in one long diagram line\n```\n"
            "Done.";
        CK(plain_lang_ste(chart, &s) == 0, "ste ok 3");
        CK(s.ste_clear == 1, "fence content skipped by STE");
    }

    /* Flagging: banned words + long sentences */
    size_t n = 0;
    plain_lang_flags("We will utilise our robust framework to "
                     "commence the transformation of the entire "
                     "streamlined administrative apparatus.", NULL, 0, &n);
    CK(n >= 3, "flags >= 3 (utilise, robust, commence, streamline...)");

    /* A CLEAR filing: short words, short sentences */
    plain_lang_score("We file. We build. We share. The penny is the "
                     "truth. We are the first ones.", &m);
    CK(m.clear == 1, "short filing is CLEAR");

    /* A DIFFICULT filing must fail the gate */
    plain_lang_score("Notwithstanding the aforementioned provisions of "
                     "the hereinbefore referenced statutory framework, "
                     "utilization of the aforementioned mechanisms "
                     "shall be deemed insufficient.", &m);
    CK(m.clear == 0, "legalese fails the gate");

    printf(fails ? "%d FAILURES\n" : "PLAIN_LANG: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
