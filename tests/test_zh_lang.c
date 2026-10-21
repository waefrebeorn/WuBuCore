/* test_zh_lang — the Chinese readability gate (GOV-10 / RND-21),
 * verified against the documented thresholds and known samples. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "zh_lang.h"

static int fails = 0;
#define CK(c, m) do { if (!(c)) { printf("FAIL %s\n", m); fails++; } } while (0)
#define NEAR(a, b) (fabs((a) - (b)) < 0.5)

/* UTF-8 byte sequences for CJK characters:
 *   你 = U+4F60 = E4 BD A0    好 = U+597D = E5 A5 BD
 *   世 = U+4E16 = E4 B8 96    界 = U+754C = E7 95 8C
 *   中 = U+4E2D = E4 B8 AD    国 = U+56FD = E5 9B BD
 *   。 = U+3002 = E3 80 82    ！ = U+FF01 = EF BC 81
 *   ？ = U+FF1F = EF BC 9F    ！ = U+FF01 = EF BC 81
 *   民 = U+6C11 = E6 B0 91    主 = U+4E3B = E4 B8 BB
 *   国 = U+56FD = E5 9B BD
 *
 * NOTE: C \\x escapes are greedy — \xef\xbc\x81 would merge into
 * \xefbc (one byte 0xBC) then \x81. We use octal escapes for the
 * 3-byte CJK punctuation to avoid greedy merging.
 */

/* "你好" = 4F60 597D → 2 chars */
static const char *HELLO2 = "\xe4\xbd\xa0\xe5\xa5\xbd";

/* "你好世界" = 4F60 597D 4E16 754C → 4 chars */
static const char *HELLO4 =
    "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c";

/* "你好。世界" = 你好。 (4F60 597D 3002) 世界 (4E16 754C) → 4 CJK chars, 2 sentences */
static const char *TWO_SENTENCES =
    "\xe4\xbd\xa0\xe5\xa5\xbd\xe3\x80\x82\xe4\xb8\x96\xe7\x95\x8c";

/* Extension character U+20599 (4-byte UTF-8: F0 A0 96 99) + 中 (4E2D)
 * U+20599 is in CJK Ext B → always difficult */
static const char *WITH_EXT =
    "\xf0\xa0\x96\x99\xe4\xb8\xad";

/* "你好！世界？" → 4 CJK chars, 2 sentences (！ and ？)
 * Using octal escapes for full-width punct to avoid C \\x greediness. */
/* ！ = U+FF01 = EF BC 81 = octal 357 274 201 */
/* ？ = U+FF1F = EF BC 9F = octal 357 274 237 */
static const char *FULLWIDTH_TERMS =
    "\xe4\xbd\xa0\xe5\xa5\xbd"      /* 你好 */
    "\357\274\201"                    /* ！ U+FF01 */
    "\xe4\xb8\x96\xe7\x95\x8c"      /* 世界 */
    "\357\274\237";                   /* ？ U+FF1F */

int main(void) {
    zh_metrics m;

    /* Test 1: basic CJK detection */
    CK(zh_lang_is_cjk(0x4E2D) == 1, "U+4E2D (中) is CJK");
    CK(zh_lang_is_cjk(0x4F60) == 1, "U+4F60 (你) is CJK");
    CK(zh_lang_is_cjk(0x6587) == 1, "U+6587 (文) is CJK");
    CK(zh_lang_is_cjk(0x3002) == 1, "U+3002 (。) is in CJK range");
    CK(zh_lang_is_cjk(0x0041) == 0, "ASCII A is not CJK");
    CK(zh_lang_is_cjk(0x002E) == 0, "ASCII period is not CJK");

    /* Test 2: simple character counting — "你好" = 2 chars */
    zh_lang_score(HELLO2, &m);
    CK(m.chars == 2, "2 chars in '你好'");
    CK(m.sentences == 1, "1 sentence (no terminator → 1)");
    CK(NEAR(m.chars_per_sentence, 2.0), "2 chars/sentence");

    /* Test 3: four characters — "你好世界" */
    zh_lang_score(HELLO4, &m);
    CK(m.chars == 4, "4 chars in '你好世界'");
    CK(m.sentences == 1, "1 sentence");

    /* Test 4: two sentences split by 。 */
    zh_lang_score(TWO_SENTENCES, &m);
    CK(m.chars == 4, "4 chars in two-sentence text");
    CK(m.sentences == 2, "2 sentences (split by 。)");
    CK(NEAR(m.chars_per_sentence, 2.0), "2 chars/sentence");

    /* Test 5: extension character is difficult */
    zh_lang_score(WITH_EXT, &m);
    CK(m.chars == 2, "2 CJK chars (1 extension + 中)");
    CK(m.difficult == 1, "1 difficult char (U+20599 extension)");
    CK(NEAR(m.difficult_ratio, 0.5), "0.5 difficult ratio");

    /* Test 6: common characters are not difficult */
    zh_lang_score(HELLO4, &m);
    CK(m.difficult == 0, "no difficult chars in 你好世界");

    /* Test 7: unique character counting */
    zh_lang_score(HELLO4, &m);
    CK(m.unique_chars == 4, "4 unique chars (all distinct)");

    /* Test 8: empty text */
    zh_lang_score("", &m);
    CK(m.chars == 0, "empty text: 0 chars");
    CK(m.sentences == 1, "empty text: 1 sentence (minimum)");

    /* Test 9: NULL rejected */
    CK(zh_lang_score(NULL, &m) == -1, "NULL text rejected");
    CK(zh_lang_score(HELLO2, NULL) == -1, "NULL output rejected");

    /* Test 10: ASCII is not counted as Chinese chars */
    zh_lang_score("Hello world test", &m);
    CK(m.chars == 0, "ASCII text: 0 Chinese chars");

    /* Test 11: mixed ASCII + Chinese */
    zh_lang_score("Hello 你好 world", &m);
    CK(m.chars == 2, "mixed: 2 Chinese chars");
    CK(m.sentences == 1, "mixed: 1 sentence");

    /* Test 12: full-width punctuation terminators (! and ?) */
    zh_lang_score(FULLWIDTH_TERMS, &m);
    CK(m.chars == 4, "4 chars with full-width ! and ?");
    CK(m.sentences == 2, "2 sentences (full-width ! and ?)");
    CK(NEAR(m.chars_per_sentence, 2.0), "2 chars/sentence");

    /* Test 13: low diversity — same char repeated 80 times */
    {
        /* Build "你" * 80 (UTF-8: E4 BD A0 repeated) */
        char buf[256];
        memset(buf, 0, sizeof(buf));
        for (int i = 0; i < 80; i++) {
            buf[i * 3] = '\xe4';
            buf[i * 3 + 1] = '\xbd';
            buf[i * 3 + 2] = '\xa0';
        }
        buf[80 * 3] = '\0';
        zh_lang_score(buf, &m);
        CK(m.chars == 80, "80 chars in low-diversity text");
        CK(m.unique_chars == 1, "only 1 unique char");
        CK(NEAR(m.char_diversity, 0.0125), "diversity ~0.0125");
        CK(m.clear == 0, "low diversity fails the gate");
    }

    /* Test 14: verdict string for no Chinese chars */
    zh_lang_score("hello world", &m);
    CK(strstr(zh_lang_verdict(&m), "NO CHINESE") != NULL,
       "verdict: no Chinese chars");

    /* Test 15: long sentence (chars_per_sentence > 40) fails gate */
    {
        /* 50 distinct common characters, no sentence terminator */
        /* Build 50 "你" characters: 150 bytes */
        char buf[200];
        memset(buf, 0, sizeof(buf));
        for (int i = 0; i < 50; i++) {
            buf[i * 3] = '\xe4';
            buf[i * 3 + 1] = '\xbd';
            buf[i * 3 + 2] = '\xa0';
        }
        buf[50 * 3] = '\0';
        zh_lang_score(buf, &m);
        CK(m.chars == 50, "50 chars in long-sentence text");
        CK(m.sentences == 1, "1 sentence (no terminator)");
        CK(m.chars_per_sentence == 50.0, "50 chars/sentence");
        CK(m.chars_per_sentence > ZH_CLEAR_SENTENCE_LEN,
           "50 > 40 threshold");
        CK(m.clear == 0, "long sentence fails the gate");
    }

    printf(fails ? "%d FAILURES\n" : "ZH_LANG: ALL PASS\n", fails);
    return fails ? 1 : 0;
}
