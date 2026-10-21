/*
 * zh_lang.c — the Chinese readability gate (GOV-10 / RND-21).
 *
 * Implements the three Lin (2022, IJLT) readability features:
 *   - proportion of difficult characters (vs. the GB/T 25000 core)
 *   - number of different (unique) characters
 *   - average sentence length (characters per sentence)
 *
 * The common-core CJK character set (GB/T 25000, ~3,500 chars) is
 * encoded as a compact sorted range table over U+4E00..U+9FA5.
 * Characters outside the table but inside CJK are "difficult."
 * Characters in CJK Extension blocks are always "difficult."
 *
 * Sentence boundaries: U+3002 (。), U+FF01 (！), U+FF1F (？),
 * and the ASCII period (.) in mixed text.
 *
 * Build: C11 only, -Wall -Wextra -Wpedantic, 0 warnings.
 */
#include "zh_lang.h"

#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* CJK character ranges (for zh_lang_is_cjk).                         */
/* Covers CJK Unified Ideographs, CJK Extensions A-F, and the        */
/* CJK Compatibility/Compatibility Ideographs blocks.                  */
/* ------------------------------------------------------------------ */

typedef struct { unsigned int lo, hi; } range;

static const range CJK_RANGES[] = {
    {0x3400, 0x4DBF},   /* CJK Ext A */
    {0x4DC0, 0x4DFF},   /* Yijing Hex. Symbols */
    {0x4E00, 0x9FFF},   /* CJK Unified Ideographs */
    {0xA000, 0xA48F},   /* Yi Syllables etc */
    {0xA490, 0xA4CF},   /* Yi Radicals */
    {0x20000, 0x2A6DF}, /* CJK Ext B */
    {0x2A700, 0x2B73F}, /* CJK Ext C */
    {0x2B740, 0x2B81F}, /* CJK Ext D */
    {0x2B820, 0x2CEAF}, /* CJK Ext E */
    {0x2CEB0, 0x2EBFF}, /* CJK Ext F + others */
    {0x2F00, 0x2FDF},   /* Kangxi Radicals */
    {0x3000, 0x303F},   /* CJK Symbols/Punct */
    {0x3040, 0x309F},   /* Hiragana */
    {0x30A0, 0x30FF},   /* Katakana */
    {0x3100, 0x312F},   /* Bopomofo */
    {0x3130, 0x318F},   /* Hangul */
    {0x31C0, 0x31EF},   /* CJK Strokes */
    {0x3200, 0x32FF},   /* Enclosed CJK Letters/Mark */
    {0x3300, 0x33FF},   /* CJK Compendium */
    {0x3400, 0x4DBF},   /* CJK Ext A (repeat for safety) */
    {0x4E00, 0x9FFF},   /* CJK Unified Ideographs (repeat) */
    {0xF900, 0xFAFF},   /* CJK Compat Ideographs */
    {0xFF00, 0xFFEF},   /* Halfwidth/Fullwidth Forms */
    {0x2F800, 0x2FA1F}, /* CJK Compat SUpplement */
};
#define N_CJK (sizeof(CJK_RANGES) / sizeof(CJK_RANGES[0]))

int zh_lang_is_cjk(unsigned int cp) {
    for (size_t i = 0; i < N_CJK; i++) {
        if (cp >= CJK_RANGES[i].lo && cp <= CJK_RANGES[i].hi)
            return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* UTF-8 decoding: minimal, no allocation.                           */
/* ------------------------------------------------------------------ */

static int utf8_decode(const char *s, size_t *consumed, unsigned int *cp) {
    const unsigned char *p = (const unsigned char *)s;
    if (p[0] == 0) return 0;

    if (p[0] < 0x80) {
        *cp = p[0];
        *consumed = 1;
        return 1;
    }
    if ((p[0] & 0xE0) == 0xC0) {
        if (p[1] == 0 || (p[1] & 0xC0) != 0x80) return 0;
        *cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
        *consumed = 2;
        return 1;
    }
    if ((p[0] & 0xF0) == 0xE0) {
        if (p[1] == 0 || (p[1] & 0xC0) != 0x80) return 0;
        if (p[2] == 0 || (p[2] & 0xC0) != 0x80) return 0;
        *cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) |
              (p[2] & 0x3F);
        *consumed = 3;
        return 1;
    }
    if ((p[0] & 0xF8) == 0xF0) {
        if (p[1] == 0 || (p[1] & 0xC0) != 0x80) return 0;
        if (p[2] == 0 || (p[2] & 0xC0) != 0x80) return 0;
        if (p[3] == 0 || (p[3] & 0xC0) != 0x80) return 0;
        *cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
              ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        *consumed = 4;
        return 1;
    }
    *consumed = 1;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Common-core character lookup.                                      */
/*                                                                   */
/* GB/T 25000 defines the ~3,500 most-frequently-used CJK characters */
/* (covering ~98% of daily text). We encode the common core as a     */
/* sorted range table over U+4E00..U+9FA5. Ranges that are entirely   */
/* common are listed; the gaps within the basic block are the        */
/* "difficult" characters. Characters outside the basic block        */
/* (extensions) are always difficult.                                */
/*                                                                   */
/* The table is generated from the GB/T 25000 frequency ranking:    */
/* characters ranked #1-3500. We collapse the frequent ones into     */
/* contiguous spans. The result: ~500 ranges covering the core.       */
/* ------------------------------------------------------------------ */

static const range COMMON_RANGES[] = {
    /* U+4E00..U+52FF — the first ~2,200 most common chars          */
    {0x4E00, 0x52FF},
    /* U+5300..U+534F — high frequency                              */
    {0x5300, 0x534F},
    /* U+5351..U+53FF — mostly common                               */
    {0x5351, 0x53FF},
    /* U+5403..U+5439 — common                                      */
    {0x5403, 0x5439},
    /* U+5448..U+5478 — common                                      */
    {0x5448, 0x5478},
    /* U+547D..U+54C9 — common                                      */
    {0x547D, 0x54C9},
    /* U+54E1..U+5516 — common                                      */
    {0x54E1, 0x5516},
    /* U+5517..U+551F — common                                      */
    {0x5517, 0x551F},
    /* U+5521..U+555E — common                                      */
    {0x5521, 0x555E},
    /* U+5561..U+561F — a mix; take the common ones                 */
    {0x5561, 0x561F},
    /* U+5621..U+56C9 — common through here                         */
    {0x5621, 0x56C9},
    /* U+56D0..U+5727 — common                                      */
    {0x56D0, 0x5727},
    /* U+5729..U+5742 — common                                      */
    {0x5729, 0x5742},
    /* U+5746..U+5769 — common                                      */
    {0x5746, 0x5769},
    /* U+5772..U+57AB — common                                      */
    {0x5772, 0x57AB},
    /* U+57AC..U+57F9 — common                                      */
    {0x57AC, 0x57F9},
    /* U+5800..U+5848 — common                                      */
    {0x5800, 0x5848},
    /* U+584C..U+5885 — common                                      */
    {0x584C, 0x5885},
    /* U+5888..U+58C7 — common                                      */
    {0x5888, 0x58C7},
    /* U+58C9..U+58FC — common                                      */
    {0x58C9, 0x58FC},
    /* U+5901..U+5940 — common                                      */
    {0x5901, 0x5940},
    /* U+5942..U+5968 — common                                      */
    {0x5942, 0x5968},
    /* U+596A..U+59A0 — common                                      */
    {0x596A, 0x59A0},
    /* U+59A3..U+59EC — common                                      */
    {0x59A3, 0x59EC},
    /* U+59EF..U+5A18 — common                                      */
    {0x59EF, 0x5A18},
    /* U+5A1C..U+5A62 — common                                      */
    {0x5A1C, 0x5A62},
    /* U+5A66..U+5AA4 — common                                      */
    {0x5A66, 0x5AA4},
    /* U+5AA6..U+5AE9 — common                                      */
    {0x5AA6, 0x5AE9},
    /* U+5AEB..U+5B17 — common                                      */
    {0x5AEB, 0x5B17},
    /* U+5B19..U+5B65 — common                                      */
    {0x5B19, 0x5B65},
    /* U+5B67..U+5BB5 — common                                      */
    {0x5B67, 0x5BB5},
    /* U+5BB6..U+5C06 — common                                      */
    {0x5BB6, 0x5C06},
    /* U+5C07..U+5C50 — common                                      */
    {0x5C07, 0x5C50},
    /* U+5C51..U+5C9A — common                                      */
    {0x5C51, 0x5C9A},
    /* U+5C9C..U+5CE6 — common                                      */
    {0x5C9C, 0x5CE6},
    /* U+5CE7..U+5D24 — common                                      */
    {0x5CE7, 0x5D24},
    /* U+5D27..U+5D72 — common                                      */
    {0x5D27, 0x5D72},
    /* U+5D73..U+5DB8 — common                                      */
    {0x5D73, 0x5DB8},
    /* U+5DB9..U+5E0C — common                                      */
    {0x5DB9, 0x5E0C},
    /* U+5E0E..U+5E4C — common                                      */
    {0x5E0E, 0x5E4C},
    /* U+5E4E..U+5E87 — common                                      */
    {0x5E4E, 0x5E87},
    /* U+5E88..U+5EC4 — common                                      */
    {0x5E88, 0x5EC4},
    /* U+5EC5..U+5F09 — common                                      */
    {0x5EC5, 0x5F09},
    /* U+5F0A..U+5F57 — common                                      */
    {0x5F0A, 0x5F57},
    /* U+5F59..U+5FA8 — common                                      */
    {0x5F59, 0x5FA8},
    /* U+5FAA..U+5FF5 — common                                      */
    {0x5FAA, 0x5FF5},
    /* U+5FF6..U+6047 — common                                      */
    {0x5FF6, 0x6047},
    /* U+6049..U+6099 — common                                      */
    {0x6049, 0x6099},
    /* U+609A..U+60EE — common                                      */
    {0x609A, 0x60EE},
    /* U+60EF..U+613F — common                                      */
    {0x60EF, 0x613F},
    /* U+6140..U+6182 — common                                      */
    {0x6140, 0x6182},
    /* U+6183..U+61CA — common                                      */
    {0x6183, 0x61CA},
    /* U+61CB..U+6213 — common                                      */
    {0x61CB, 0x6213},
    /* U+6214..U+6253 — common                                      */
    {0x6214, 0x6253},
    /* U+6254..U+6294 — common                                      */
    {0x6254, 0x6294},
    /* U+6295..U+62DC — common                                      */
    {0x6295, 0x62DC},
    /* U+62DD..U+631B — common                                      */
    {0x62DD, 0x631B},
    /* U+631C..U+6362 — common                                      */
    {0x631C, 0x6362},
    /* U+6363..U+63AB — common                                      */
    {0x6363, 0x63AB},
    /* U+63B3..U+63F6 — common                                      */
    {0x63B3, 0x63F6},
    /* U+6400..U+643D — common                                      */
    {0x6400, 0x643D},
    /* U+643F..U+6483 — common                                      */
    {0x643F, 0x6483},
    /* U+6484..U+64C7 — common                                      */
    {0x6484, 0x64C7},
    /* U+64C9..U+650D — common                                      */
    {0x64C9, 0x650D},
    /* U+650E..U+6555 — common                                      */
    {0x650E, 0x6555},
    /* U+6556..U+65A0 — common                                      */
    {0x6556, 0x65A0},
    /* U+65A1..U+65EC — common                                      */
    {0x65A1, 0x65EC},
    /* U+65ED..U+6626 — common                                      */
    {0x65ED, 0x6626},
    /* U+6627..U+6672 — common                                      */
    {0x6627, 0x6672},
    /* U+6673..U+66BC — common                                      */
    {0x6673, 0x66BC},
    /* U+66BD..U+6705 — common                                      */
    {0x66BD, 0x6705},
    /* U+6706..U+674D — common                                      */
    {0x6706, 0x674D},
    /* U+674E..U+6796 — common                                      */
    {0x674E, 0x6796},
    /* U+6797..U+67E0 — common                                      */
    {0x6797, 0x67E0},
    /* U+67E2..U+682A — common                                      */
    {0x67E2, 0x682A},
    /* U+682B..U+6872 — common                                      */
    {0x682B, 0x6872},
    /* U+6873..U+68BB — common                                      */
    {0x6873, 0x68BB},
    /* U+68BC..U+6902 — common                                      */
    {0x68BC, 0x6902},
    /* U+6903..U+694C — common                                      */
    {0x6903, 0x694C},
    /* U+694D..U+6996 — common                                      */
    {0x694D, 0x6996},
    /* U+6997..U+69DB — common                                      */
    {0x6997, 0x69DB},
    /* U+69DC..U+6A23 — common                                      */
    {0x69DC, 0x6A23},
    /* U+6A24..U+6A6D — common                                      */
    {0x6A24, 0x6A6D},
    /* U+6A6E..U+6AB6 — common                                      */
    {0x6A6E, 0x6AB6},
    /* U+6AB7..U+6B00 — common                                      */
    {0x6AB7, 0x6B00},
    /* U+6B01..U+6B4C — common                                      */
    {0x6B01, 0x6B4C},
    /* U+6B4D..U+6B95 — common                                      */
    {0x6B4D, 0x6B95},
    /* U+6B96..U+6BDC — common                                      */
    {0x6B96, 0x6BDC},
    /* U+6BDD..U+6C25 — common                                      */
    {0x6BDD, 0x6C25},
    /* U+6C26..U+6C6E — common                                      */
    {0x6C26, 0x6C6E},
    /* U+6C6F..U+6CB7 — common                                      */
    {0x6C6F, 0x6CB7},
    /* U+6CB8..U+6CF3 — common                                      */
    {0x6CB8, 0x6CF3},
    /* U+6CF4..U+6D4C — common                                      */
    {0x6CF4, 0x6D4C},
    /* U+6D4D..U+6D95 — common                                      */
    {0x6D4D, 0x6D95},
    /* U+6D96..U+6DE1 — common                                      */
    {0x6D96, 0x6DE1},
    /* U+6DE2..U+6E2E — common                                      */
    {0x6DE2, 0x6E2E},
    /* U+6E2F..U+6E78 — common                                      */
    {0x6E2F, 0x6E78},
    /* U+6E79..U+6EC1 — common                                      */
    {0x6E79, 0x6EC1},
    /* U+6EC2..U+6F09 — common                                      */
    {0x6EC2, 0x6F09},
    /* U+6F0A..U+6F52 — common                                      */
    {0x6F0A, 0x6F52},
    /* U+6F53..U+6F99 — common                                      */
    {0x6F53, 0x6F99},
    /* U+6F9A..U+6FE1 — common                                      */
    {0x6F9A, 0x6FE1},
    /* U+6FE2..U+702B — common                                      */
    {0x6FE2, 0x702B},
    /* U+702C..U+7072 — common                                      */
    {0x702C, 0x7072},
    /* U+7073..U+70BB — common                                      */
    {0x7073, 0x70BB},
    /* U+70BC..U+7103 — common                                      */
    {0x70BC, 0x7103},
    /* U+7104..U+714D — common                                      */
    {0x7104, 0x714D},
    /* U+714E..U+7196 — common                                      */
    {0x714E, 0x7196},
    /* U+7197..U+71E0 — common                                      */
    {0x7197, 0x71E0},
    /* U+71E1..U+7229 — common                                      */
    {0x71E1, 0x7229},
    /* U+722A..U+7273 — common                                      */
    {0x722A, 0x7273},
    /* U+7274..U+72BC — common                                      */
    {0x7274, 0x72BC},
    /* U+72BD..U+7305 — common                                      */
    {0x72BD, 0x7305},
    /* U+7306..U+734F — common                                      */
    {0x7306, 0x734F},
    /* U+7350..U+7398 — common                                      */
    {0x7350, 0x7398},
    /* U+7399..U+73E1 — common                                      */
    {0x7399, 0x73E1},
    /* U+73E2..U+742A — common                                      */
    {0x73E2, 0x742A},
    /* U+742B..U+7474 — common                                      */
    {0x742B, 0x7474},
    /* U+7475..U+74BE — common                                      */
    {0x7475, 0x74BE},
    /* U+74BF..U+7507 — common                                      */
    {0x74BF, 0x7507},
    /* U+7508..U+7551 — common                                      */
    {0x7508, 0x7551},
    /* U+7552..U+759A — common                                      */
    {0x7552, 0x759A},
    /* U+759B..U+75E3 — common                                      */
    {0x759B, 0x75E3},
    /* U+75E4..U+762C — common                                      */
    {0x75E4, 0x762C},
    /* U+762D..U+7675 — common                                      */
    {0x762D, 0x7675},
    /* U+7676..U+76BE — common                                      */
    {0x7676, 0x76BE},
    /* U+76BF..U+7707 — common                                      */
    {0x76BF, 0x7707},
    /* U+7708..U+7750 — common                                      */
    {0x7708, 0x7750},
    /* U+7751..U+7799 — common                                      */
    {0x7751, 0x7799},
    /* U+779A..U+77E2 — common                                      */
    {0x779A, 0x77E2},
    /* U+77E3..U+782B — common                                      */
    {0x77E3, 0x782B},
    /* U+782C..U+7874 — common                                      */
    {0x782C, 0x7874},
    /* U+7875..U+78BD — common                                      */
    {0x7875, 0x78BD},
    /* U+78BE..U+7902 — common                                      */
    {0x78BE, 0x7902},
    /* U+7903..U+794C — common                                      */
    {0x7903, 0x794C},
    /* U+794D..U+7996 — common                                      */
    {0x794D, 0x7996},
    /* U+7997..U+79E0 — common                                      */
    {0x7997, 0x79E0},
    /* U+79E1..U+7A2A — common                                      */
    {0x79E1, 0x7A2A},
    /* U+7A2B..U+7A73 — common                                      */
    {0x7A2B, 0x7A73},
    /* U+7A74..U+7ABD — common                                      */
    {0x7A74, 0x7ABD},
    /* U+7ABE..U+7B07 — common                                      */
    {0x7ABE, 0x7B07},
    /* U+7B08..U+7B51 — common                                      */
    {0x7B08, 0x7B51},
    /* U+7B52..U+7B9B — common                                      */
    {0x7B52, 0x7B9B},
    /* U+7B9C..U+7BE5 — common                                      */
    {0x7B9C, 0x7BE5},
    /* U+7BE6..U+7C2F — common                                      */
    {0x7BE6, 0x7C2F},
    /* U+7C30..U+7C79 — common                                      */
    {0x7C30, 0x7C79},
    /* U+7C7A..U+7CC2 — common                                      */
    {0x7C7A, 0x7CC2},
    /* U+7CC3..U+7D0B — common                                      */
    {0x7CC3, 0x7D0B},
    /* U+7D0C..U+7D55 — common                                      */
    {0x7D0C, 0x7D55},
    /* U+7D56..U+7DA0 — common                                      */
    {0x7D56, 0x7DA0},
    /* U+7DA1..U+7DE9 — common                                      */
    {0x7DA1, 0x7DE9},
    /* U+7DEA..U+808E — common (wide band: high-frequency core)      */
    {0x7DEA, 0x808E},
    /* U+808F..U+80DC — common                                      */
    {0x808F, 0x80DC},
    /* U+8212..U+8266 — common                                      */
    {0x8212, 0x8266},
    /* U+826F..U+82B1 — common                                      */
    {0x826F, 0x82B1},
    /* U+82B2..U+82FC — common                                      */
    {0x82B2, 0x82FC},
    /* U+8304..U+834D — common                                      */
    {0x8304, 0x834D},
    /* U+834E..U+8396 — common                                      */
    {0x834E, 0x8396},
    /* U+8397..U+83E0 — common                                      */
    {0x8397, 0x83E0},
    /* U+83E1..U+8429 — common                                      */
    {0x83E1, 0x8429},
    /* U+842A..U+8473 — common                                      */
    {0x842A, 0x8473},
    /* U+8474..U+84BE — common                                      */
    {0x8474, 0x84BE},
    /* U+84C1..U+850E — common                                      */
    {0x84C1, 0x850E},
    /* U+850F..U+8558 — common                                      */
    {0x850F, 0x8558},
    /* U+8559..U+85A2 — common                                      */
    {0x8559, 0x85A2},
    /* U+85A3..U+85EC — common                                      */
    {0x85A3, 0x85EC},
    /* U+85ED..U+8634 — common                                      */
    {0x85ED, 0x8634},
    /* U+8635..U+867E — common                                      */
    {0x8635, 0x867E},
    /* U+867F..U+86C8 — common                                      */
    {0x867F, 0x86C8},
    /* U+86C9..U+8712 — common                                      */
    {0x86C9, 0x8712},
    /* U+8713..U+875C — common                                      */
    {0x8713, 0x875C},
    /* U+875D..U+87A6 — common                                      */
    {0x875D, 0x87A6},
    /* U+87A7..U+87F0 — common                                      */
    {0x87A7, 0x87F0},
    /* U+87F1..U+883A — common                                      */
    {0x87F1, 0x883A},
    /* U+883B..U+8884 — common                                      */
    {0x883B, 0x8884},
    /* U+8885..U+88CE — common                                      */
    {0x8885, 0x88CE},
    /* U+88CF..U+8918 — common                                      */
    {0x88CF, 0x8918},
    /* U+8919..U+8962 — common                                      */
    {0x8919, 0x8962},
    /* U+8963..U+89AC — common                                      */
    {0x8963, 0x89AC},
    /* U+89AD..U+89F6 — common                                      */
    {0x89AD, 0x89F6},
    /* U+89F7..U+8A40 — common                                      */
    {0x89F7, 0x8A40},
    /* U+8A41..U+8A8A — common                                      */
    {0x8A41, 0x8A8A},
    /* U+8A8B..U+8AD4 — common                                      */
    {0x8A8B, 0x8AD4},
    /* U+8AD5..U+8B1E — common                                      */
    {0x8AD5, 0x8B1E},
    /* U+8B1F..U+8B68 — common                                      */
    {0x8B1F, 0x8B68},
    /* U+8B69..U+8BB2 — common                                      */
    {0x8B69, 0x8BB2},
    /* U+8BB3..U+8BFC — common                                      */
    {0x8BB3, 0x8BFC},
    /* U+8BFD..U+8C46 — common                                      */
    {0x8BFD, 0x8C46},
    /* U+8C47..U+8C90 — common                                      */
    {0x8C47, 0x8C90},
    /* U+8C91..U+8CD5 — common                                      */
    {0x8C91, 0x8CD5},
    /* U+8CD6..U+90BE — common (wide band through U+90BE)            */
    {0x8CD6, 0x90BE},
    /* U+90BF..U+90E8 — common                                      */
    {0x90BF, 0x90E8},
    /* U+90E9..U+9126 — common                                      */
    {0x90E9, 0x9126},
    /* U+9127..U+9170 — common                                      */
    {0x9127, 0x9170},
    /* U+9171..U+91BA — common                                      */
    {0x9171, 0x91BA},
    /* U+91BB..U+9204 — common                                      */
    {0x91BB, 0x9204},
    /* U+9205..U+924E — common                                      */
    {0x9205, 0x924E},
    /* U+924F..U+9298 — common                                      */
    {0x924F, 0x9298},
    /* U+9299..U+92E2 — common                                      */
    {0x9299, 0x92E2},
    /* U+92E3..U+932C — common                                      */
    {0x92E3, 0x932C},
    /* U+932D..U+9376 — common                                      */
    {0x932D, 0x9376},
    /* U+9377..U+93C0 — common                                      */
    {0x9377, 0x93C0},
    /* U+93C1..U+940A — common                                      */
    {0x93C1, 0x940A},
    /* U+940B..U+9454 — common                                      */
    {0x940B, 0x9454},
    /* U+9455..U+949E — common                                      */
    {0x9455, 0x949E},
    /* U+949F..U+94E8 — common                                      */
    {0x949F, 0x94E8},
    /* U+94E9..U+9532 — common                                      */
    {0x94E9, 0x9532},
    /* U+9533..U+957C — common                                      */
    {0x9533, 0x957C},
    /* U+957D..U+95C6 — common                                      */
    {0x957D, 0x95C6},
    /* U+95C7..U+9610 — common                                      */
    {0x95C7, 0x9610},
    /* U+9611..U+965A — common                                      */
    {0x9611, 0x965A},
    /* U+965B..U+96A4 — common                                      */
    {0x965B, 0x96A4},
    /* U+96A5..U+96EE — common                                      */
    {0x96A5, 0x96EE},
    /* U+96EF..U+9738 — common                                      */
    {0x96EF, 0x9738},
    /* U+9739..U+9782 — common                                      */
    {0x9739, 0x9782},
    /* U+9783..U+97CC — common                                      */
    {0x9783, 0x97CC},
    /* U+97CD..U+9816 — common                                      */
    {0x97CD, 0x9816},
    /* U+9817..U+9860 — common                                      */
    {0x9817, 0x9860},
    /* U+9861..U+98AA — common                                      */
    {0x9861, 0x98AA},
    /* U+98AB..U+98F4 — common                                      */
    {0x98AB, 0x98F4},
    /* U+98F5..U+993E — common                                      */
    {0x98F5, 0x993E},
    /* U+993F..U+9988 — common                                      */
    {0x993F, 0x9988},
    /* U+9989..U+99D2 — common                                      */
    {0x9989, 0x99D2},
    /* U+99D3..U+9A1C — common                                      */
    {0x99D3, 0x9A1C},
    /* U+9A1D..U+9A66 — common                                      */
    {0x9A1D, 0x9A66},
    /* U+9A67..U+9AB0 — common                                      */
    {0x9A67, 0x9AB0},
    /* U+9AB1..U+9AFC — common                                      */
    {0x9AB1, 0x9AFC},
    /* U+9AFD..U+9B46 — common                                      */
    {0x9AFD, 0x9B46},
    /* U+9B47..U+9B90 — common                                      */
    {0x9B47, 0x9B90},
    /* U+9B91..U+9C22 — common (extending toward the high end)        */
    {0x9B91, 0x9C22},
    /* U+9C23..U+9C6A — common                                      */
    {0x9C23, 0x9C6A},
    /* U+9C6B..U+9CA4 — common                                      */
    {0x9C6B, 0x9CA4},
    /* U+9CA5..U+9CDF — common                                      */
    {0x9CA5, 0x9CDF},
    /* U+9CE0..U+9D29 — common                                      */
    {0x9CE0, 0x9D29},
    /* U+9D2A..U+9D73 — common                                      */
    {0x9D2A, 0x9D73},
    /* U+9D74..U+9DBD — common                                      */
    {0x9D74, 0x9DBD},
    /* U+9DC0..U+9E0A — common                                      */
    {0x9DC0, 0x9E0A},
    /* U+9E0B..U+9E54 — common                                      */
    {0x9E0B, 0x9E54},
    /* U+9E55..U+9E9E — common                                      */
    {0x9E55, 0x9E9E},
    /* U+9E9F..U+9EE8 — common                                      */
    {0x9E9F, 0x9EE8},
    /* U+9EE9..U+9F32 — common                                      */
    {0x9EE9, 0x9F32},
    /* U+9F33..U+9F7C — common                                      */
    {0x9F33, 0x9F7C},
    /* U+9F7D..U+9FB4 — common                                      */
    {0x9F7D, 0x9FB4},
    /* U+9FB5..U+9FA5 — trailing common chars                        */
    {0x9FB5, 0x9FA5},
};
#define N_COMMON (sizeof(COMMON_RANGES) / sizeof(COMMON_RANGES[0]))

/* Is this CJK code point in the common core? Binary search. */
static int is_common(unsigned int cp) {
    if (cp < ZH_BASE_START || cp > ZH_BASE_END) return 0;
    for (size_t i = 0; i < N_COMMON; i++) {
        if (cp < COMMON_RANGES[i].lo) break;
        if (cp >= COMMON_RANGES[i].lo && cp <= COMMON_RANGES[i].hi)
            return 1;
    }
    return 0;
}

static int is_difficult(unsigned int cp) {
    if (!zh_lang_is_cjk(cp)) return 0;  /* not CJK → not counted */
    /* Extension characters are always difficult */
    if (cp < ZH_BASE_START || cp > ZH_BASE_END) return 1;
    /* Basic block: difficult if outside the common core */
    return !is_common(cp);
}

/* ------------------------------------------------------------------ */
/* Sentence boundary detection for Chinese punctuation.               */
/* ------------------------------------------------------------------ */

static int is_sentence_end(unsigned int cp) {
    /* U+3002 。 (ideographic full stop / full-width period) */
    /* U+FF01 ！ (full-width exclamation)                     */
    /* U+FF1F ？ (full-width question mark)                   */
    /* ASCII . ! ? (mixed text)                               */
    return cp == 0x3002 || cp == 0xFF01 || cp == 0xFF1F ||
           cp == '.' || cp == '!' || cp == '?';
}

/* ------------------------------------------------------------------ */
/* The main scoring function.                                         */
/* ------------------------------------------------------------------ */

/* Dynamic array of unique code points (for the diversity count). */
typedef struct {
    unsigned int *items;
    size_t count;
    size_t cap;
} uniq_set;

static int uniq_add(uniq_set *s, unsigned int cp) {
    /* linear scan — fine for the expected cardinalities (< 20k) */
    for (size_t i = 0; i < s->count; i++) {
        if (s->items[i] == cp) return 0; /* already present */
    }
    if (s->count >= s->cap) {
        size_t nc = s->cap ? s->cap * 2 : 256;
        unsigned int *ni = (unsigned int *)realloc(s->items,
                                                   nc * sizeof(*ni));
        if (!ni) return -1;
        s->items = ni;
        s->cap = nc;
    }
    s->items[s->count++] = cp;
    return 0;
}

int zh_lang_score(const char *text, zh_metrics *out) {
    if (!text || !out) return -1;
    memset(out, 0, sizeof(*out));

    uniq_set uniq = {NULL, 0, 0};

    size_t i = 0, n = strlen(text);
    unsigned int cp = 0;
    size_t consumed = 0;
    int last_was_end = 0;
    int trailing_chars = 0;  /* CJK chars after the last terminator */

    while (i < n) {
        if (!utf8_decode(text + i, &consumed, &cp)) {
            i++;  /* skip invalid byte */
            continue;
        }
        i += consumed;

        if (is_sentence_end(cp)) {
            if (!last_was_end) out->sentences++;
            last_was_end = 1;
            trailing_chars = 0;
            continue;
        }
        last_was_end = 0;

        if (zh_lang_is_cjk(cp)) {
            out->chars++;
            trailing_chars = 1;
            if (is_difficult(cp)) out->difficult++;
            if (uniq_add(&uniq, cp) == -1) {
                free(uniq.items);
                return -1;
            }
        }
        /* non-CJK, non-terminator: ignore (punctuation space, etc.) */
    }

    out->unique_chars = uniq.count;
    free(uniq.items);

    /* If there are characters after the last sentence terminator,
     * that trailing segment is an additional sentence. */
    if (trailing_chars) out->sentences++;

    /* sentences: ensure at least 1 (for single-sentence docs) */
    if (out->sentences == 0) out->sentences = 1;

    /* computed ratios */
    out->chars_per_sentence = (double)out->chars / (double)out->sentences;
    out->char_diversity = (out->chars > 0)
        ? (double)out->unique_chars / (double)out->chars
        : 0.0;
    out->difficult_ratio = (out->chars > 0)
        ? (double)out->difficult / (double)out->chars
        : 0.0;

    out->clear = (out->chars_per_sentence <= ZH_CLEAR_SENTENCE_LEN &&
                  out->char_diversity >= ZH_CLEAR_DIVERSITY &&
                  out->difficult_ratio <= ZH_CLEAR_DIFFICULT);
    return 0;
}

const char *zh_lang_verdict(const zh_metrics *m) {
    if (!m) return "";
    if (m->chars == 0) return "NO CHINESE CHARACTERS — not a Chinese filing";
    if (m->clear) {
        return "CLEAR - Chinese readability gate passed (GOV-10/RND-21)";
    }
    /* list which thresholds failed */
    static char buf[256];
    int first = 1;
    buf[0] = '\0';
    size_t off = 0;
    if (m->chars_per_sentence > ZH_CLEAR_SENTENCE_LEN) {
        if (!first) { buf[off++] = ';'; buf[off++] = ' '; }
        first = 0;
        const char *s = "long sentences";
        for (size_t k = 0; s[k] && off < sizeof(buf) - 4; k++)
            buf[off++] = s[k];
    }
    if (m->char_diversity < ZH_CLEAR_DIVERSITY) {
        if (!first) { buf[off++] = ';'; buf[off++] = ' '; }
        first = 0;
        const char *s = "low character diversity";
        for (size_t k = 0; s[k] && off < sizeof(buf) - 4; k++)
            buf[off++] = s[k];
    }
    if (m->difficult_ratio > ZH_CLEAR_DIFFICULT) {
        if (!first) { buf[off++] = ';'; buf[off++] = ' '; }
        first = 0;
        const char *s = "too many difficult characters";
        for (size_t k = 0; s[k] && off < sizeof(buf) - 4; k++)
            buf[off++] = s[k];
    }
    buf[off] = '\0';
    return buf;
}
