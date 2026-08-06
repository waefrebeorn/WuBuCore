/*
 * wubusha256.h — SHA-256, self-contained C11 (no third party).
 * Clean-room implementation of FIPS 180-4.
 */
#ifndef WUBUSHA256_H
#define WUBUSHA256_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WUBU_SHA256_HEX 65 /* 64 hex chars + NUL */

/* Compute SHA-256 of data[0..len), write lowercase hex into out
 * (caller buffer >= WUBU_SHA256_HEX). Returns 0 ok, -1 on bad args. */
int wubu_sha256_hex(const unsigned char *data, size_t len, char *out);

#ifdef __cplusplus
}
#endif

#endif /* WUBUSHA256_H */
