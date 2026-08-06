#include "wid.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Checksum (RND/WID spec): the 6 digits YY NNNN are weighted
 * right-to-left x2, x3, x4, x5, x6, x7; sum mod 11 gives the
 * check digit (0-9, 'X' for 10). Matches the verified Python tool.
 */
static const int WEIGHTS[6] = {7, 6, 5, 4, 3, 2}; /* left-to-right */

struct wid {
    char country[3]; /* 2 letters + NUL */
    int year;
    int seq;
    int check;
};

int wid_check_digit(int year, int seq) {
    int digits[6];
    digits[0] = (year / 10) % 10;
    digits[1] = year % 10;
    digits[2] = (seq / 1000) % 10;
    digits[3] = (seq / 100) % 10;
    digits[4] = (seq / 10) % 10;
    digits[5] = seq % 10;

    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += digits[i] * WEIGHTS[i];
    }
    return sum % 11; /* 0-10; 10 rendered as 'X' */
}

static char check_char(int c) {
    return (c == 10) ? 'X' : (char)('0' + c);
}

static int char_to_check(char c) {
    if (c == 'X' || c == 'x') return 10;
    if (c >= '0' && c <= '9') return c - '0';
    return -1;
}

int wid_generate(const char *mind_country, int year, int seq,
                 char *out, size_t outsz) {
    if (!mind_country || strlen(mind_country) != 2 ||
        !isalpha((unsigned char)mind_country[0]) ||
        !isalpha((unsigned char)mind_country[1]) ||
        year < 0 || year > 99 || seq < 0 || seq > 9999 || !out) {
        return -1;
    }
    int check = wid_check_digit(year, seq);
    int n = snprintf(out, outsz, "WID-%c%c-%02d-%04d-%c",
                     (char)toupper((unsigned char)mind_country[0]),
                     (char)toupper((unsigned char)mind_country[1]),
                     year, seq, check_char(check));
    if (n < 0 || (size_t)n >= outsz) return -1;
    return 0;
}

int wid_validate(const char *s) {
    /* WID-XX-YY-NNNN-C : 16 chars, dashes at 6, 9, 14 */
    if (!s) return 0;
    if (strlen(s) != 16) return 0;
    if (strncmp(s, "WID-", 4) != 0) return 0;
    if (s[6] != '-' || s[9] != '-' || s[14] != '-') return 0;

    if (!isalpha((unsigned char)s[4]) || !isalpha((unsigned char)s[5])) return 0;
    /* YY at 7,8; NNNN at 10,11,12,13 */
    if (!isdigit((unsigned char)s[7]) || !isdigit((unsigned char)s[8])) return 0;
    for (int i = 10; i <= 13; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }

    int year = (s[7] - '0') * 10 + (s[8] - '0');
    int seq = (s[10] - '0') * 1000 + (s[11] - '0') * 100 +
              (s[12] - '0') * 10 + (s[13] - '0');

    int expected = wid_check_digit(year, seq);
    int given = char_to_check(s[15]);
    if (given < 0) return 0;
    return expected == given;
}

wid *wid_parse(const char *s) {
    if (!wid_validate(s)) return NULL;
    wid *w = (wid *)calloc(1, sizeof(wid));
    if (!w) return NULL;
    w->country[0] = (char)toupper((unsigned char)s[4]);
    w->country[1] = (char)toupper((unsigned char)s[5]);
    w->year = (s[7] - '0') * 10 + (s[8] - '0');
    w->seq = (s[10] - '0') * 1000 + (s[11] - '0') * 100 +
             (s[12] - '0') * 10 + (s[13] - '0');
    w->check = char_to_check(s[15]);
    return w;
}

void wid_free(wid *w) { free(w); }

const char *wid_country(const wid *w) { return w ? w->country : ""; }
int wid_year(const wid *w) { return w ? w->year : -1; }
int wid_seq(const wid *w) { return w ? w->seq : -1; }
int wid_check(const wid *w) { return w ? w->check : -1; }
