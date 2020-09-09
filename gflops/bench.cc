#include <time.h>
#include <stdio.h>

#define LOOP (1e9)

extern "C" {
    void FmlaLoop12(int);
}

static double get_time(struct timespec *start,
                       struct timespec *end) {
    return end->tv_sec - start->tv_sec + (end->tv_nsec - start->tv_nsec) * 1e-9;
}

int main() {
    struct timespec start, end;
    double time_used = 0.0;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    FmlaLoop12(LOOP);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    time_used     = get_time(&start, &end);     // seconds
    int op_floats = 12 * 4 * 2;                 // 12 mla_op (1 mla_op = 4 mul_op + 4 add_op)
    printf("perf: %.6lf GFLOPS\r\n", LOOP * op_floats * 1e-9 / time_used);
}
