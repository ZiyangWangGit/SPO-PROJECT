#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "vol.h"

int sum_sample(int16_t *buff, size_t samples) {
    int t = 0;
    for (int x = 0; x < samples; x++) {
        t += buff[x];
    }
    return t;
}

// Original function
CLONE_ATTRIBUTE
void scale_samples(int16_t *in, int16_t *out, int cnt, int volume) {
    for (int x = 0; x < cnt; x++) {
        out[x] = ((((int32_t)in[x]) * ((int32_t)(32767 * volume / 100) << 1)) >> 16);
    }
}

// Exact clone
CLONE_ATTRIBUTE
void scale_samples_v2(int16_t *in, int16_t *out, int cnt, int volume) {
    for (int x = 0; x < cnt; x++) {
        out[x] = ((((int32_t)in[x]) * ((int32_t)(32767 * volume / 100) << 1)) >> 16);
    }
}

int main() {
    int ttl = 0, ttl2 = 0;

    int16_t *in = (int16_t*)calloc(SAMPLES, sizeof(int16_t));
    int16_t *out = (int16_t*)calloc(SAMPLES, sizeof(int16_t));
    int16_t *out2 = (int16_t*)calloc(SAMPLES, sizeof(int16_t));

    vol_createsample(in, SAMPLES);

    scale_samples(in, out, SAMPLES, VOLUME);
    scale_samples_v2(in, out2, SAMPLES, VOLUME);

    ttl = sum_sample(out, SAMPLES);
    ttl2 = sum_sample(out2, SAMPLES);

    printf("Result v1: %d\n", ttl);
    printf("Result v2: %d\n", ttl2);

    free(in); free(out); free(out2);
    return 0;
}
