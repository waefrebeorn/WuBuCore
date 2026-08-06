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
     * (documented in the Flesch-Kincaid literature). */
    plain_lang_score("The Australian platypus is seemingly a hybrid of "
                     "a mammal and reptilian creature.", &m);
    CK(m.words == 13, "platypus 13 words");
    CK(NEAR(m.grade, 11.3), "platypus grade ~11.3 (documented)");

    /* GOV.UK banned word detected + replacement */
    CK(plain_lang_banned("utilise") == 1, "utilise banned");
    CK(strcmp(plain_lang_replacement("utilise"), "use") == 0, "replacement");
    CK(plain_lang_banned("use") == 0, "use is fine");
    CK(plain_lang_banned("robust") == 1, "robust banned");

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
