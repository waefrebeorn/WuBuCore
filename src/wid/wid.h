/*
 * wid.h — WID: WuBu IDentification Number (mod-11 self-verifying).
 * Opaque struct; callers depend on the interface, never the layout.
 */
#ifndef WID_H
#define WID_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A parsed/issued WID. Layout is private; use the accessors. */
typedef struct wid wid;

/* Maximum formatted length of "WID-XX-YY-NNNN-C" plus NUL */
#define WID_MAX 20

/* Generate a WID into out (caller buffer >= WID_MAX). Returns 0 ok, -1 bad. */
int wid_generate(const char *mind_country, int year, int seq,
                 char *out, size_t outsz);

/* Validate a WID string. Returns 1 valid, 0 invalid. */
int wid_validate(const char *s);

/* Parse a valid WID; returns NULL on invalid input. */
wid *wid_parse(const char *s);

void wid_free(wid *w);

const char *wid_country(const wid *w);
int wid_year(const wid *w);
int wid_seq(const wid *w);
int wid_check(const wid *w);

/* Compute the mod-11 check digit for a 6-digit payload (yy+nnnn). */
int wid_check_digit(int year, int seq);

#ifdef __cplusplus
}
#endif

#endif /* WID_H */
