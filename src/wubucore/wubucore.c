#include "wubucore.h"

#include <math.h>
#include <stdlib.h>

double wubu_clamp(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

double wubu_sigmoid(double x) {
    if (x < -10.0) return 0.0;
    if (x > 10.0) return 1.0;
    return 1.0 / (1.0 + exp(-x));
}

double wubu_tanh(double x) {
    return tanh(x);
}

unsigned long long wubu_fnv1a(const unsigned char *data, size_t len) {
    unsigned long long h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; i++) {
        h ^= (unsigned long long)data[i];
        h *= 1099511628211ULL;
    }
    return h;
}

unsigned long long wubu_rand64(unsigned long long *state) {
    unsigned long long x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

double wubu_atod(const char *s) {
    return atof(s);
}
