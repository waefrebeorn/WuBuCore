/*
 * wubucore.h — the single shared utility surface of WuBuCore.
 * Everything every module needs, in one small self-contained lib.
 * No god headers: this is the ONLY cross-module dependency.
 */
#ifndef WUBUCORE_H
#define WUBUCORE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* clamp v into [lo, hi] */
double wubu_clamp(double v, double lo, double hi);

/* logistic sigmoid */
double wubu_sigmoid(double x);

/* tanh (via math.h) with safe domain */
double wubu_tanh(double x);

/* FNV-1a 64-bit hash of bytes (for stable string hashing) */
unsigned long long wubu_fnv1a(const unsigned char *data, size_t len);

/* deterministic xorshift64* PRNG (no libc rand) */
unsigned long long wubu_rand64(unsigned long long *state);

/* parse a double from a string, 0 on failure */
double wubu_atod(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* WUBUCORE_H */
