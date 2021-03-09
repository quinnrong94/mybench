#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define LOOP (1e9)

extern "C" {
    void FmlaLoop16(int);
}

static double get_time(struct timespec *start,
                       struct timespec *end) {
    return end->tv_sec - start->tv_sec + (end->tv_nsec - start->tv_nsec) * 1e-9;
}

void run() {
    struct timespec start, end;
    double time_used = 0.0;

    // int loop = id < 4 ? LOOP / 2 : LOOP * 2;
    int loop = LOOP;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    _Pragma("omp parallel for")
    for (int i = 0; i < 16; ++i) {
        FmlaLoop16(loop/16);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    time_used     = get_time(&start, &end);     // seconds
    int op_floats = 16 * 4 * 2;                 // 16 mla_op (1 mla_op = 4 mul_op + 4 add_op)
    printf("loop %12d: %.6lf GFLOPS\r\n", loop, loop * 1.0 * op_floats * 1e-9 / time_used);
}

int main(int argc, char **argv) {
    int num_threads = 1;
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }
    printf("\n>> omp_set_num_threads(%d)\n", num_threads);
    omp_set_num_threads(num_threads);

    printf("omp_get_num_procs %d\n", omp_get_num_procs());
    printf("omp_get_max_threads %d\n", omp_get_max_threads());
    printf("omp_get_thread_num %d\n", omp_get_thread_num());

    run();

    return 0;
}
