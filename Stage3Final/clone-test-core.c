#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

CLONE_ATTRIBUTE
float process_floats(float *data, size_t len) {
    float sum = 0.0f;
    size_t i = 0;
    while (i < len) {
        sum += (data[i] * 1.2f) + (data[i] / 5.0f);
        i++;
    }
    return sum;
}

CLONE_ATTRIBUTE
int process_int32(int32_t *data, size_t len) {
    int result = 0;
    size_t i = 0;
    while (i < len) {
        result += (data[i] >> 1) + (data[i] & 0x1F);
        i++;
    }
    return result;
}

int main() {
    float b[128];
    int32_t c[128];

    // Initialize with sample values
    for (int i = 0; i < 128; ++i) {
        b[i] = (float)(i + 1);
        c[i] = i + 1;
    }

    printf("process_floats: %.2f\n", process_floats(b, 128));
    printf("process_int32: %d\n", process_int32(c, 128));

    return 0;
}
