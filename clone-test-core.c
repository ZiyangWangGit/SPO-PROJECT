#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "vol.h"

int sum_sample (int16_t *buff, size_t samples) {
  int t;
  for (int x = 0; x < SAMPLES; x++) {
    t += buff[x];
  }
  return t;
}

CLONE_ATTRIBUTE
void scale_samples(int16_t *in, int16_t *out, int cnt, int volume) {
        for (int x = 0; x < cnt; x++) {
                out[x] = ((((int32_t) in[x]) * ((int32_t) (32767 * volume / 100) <<1) ) >> 16);
        }
}

CLONE_ATTRIBUTE
void scale_samples2(int16_t *in, int16_t *out, int cnt, int volume) {
    for (int x = 0; x < cnt; x++) {
        out[x] = ((((int32_t) in[x]) * ((int32_t) (32767 * volume / 100) <<1) ) >> 16);
    }
}

int main() {
        int             x;
        int             ttl=0;

// ---- Create in[] and out[] arrays
        int16_t*        in;
        int16_t*        out;
        in =  (int16_t*) calloc(SAMPLES, sizeof(int16_t));
        out = (int16_t*) calloc(SAMPLES, sizeof(int16_t));

// ---- Create dummy samples in in[]
        vol_createsample(in, SAMPLES);

// ---- This is the part we're interested in!
// ---- Scale the samples from in[], placing results in out[]
        scale_samples(in, out, SAMPLES, VOLUME);
        scale_samples2(in, out, SAMPLES, VOLUME);
// ---- This part sums the samples.
        ttl=sum_sample(out, SAMPLES);

// ---- Print the sum of the samples.
        printf("Result: %d\n", ttl);

        return 0;

}
