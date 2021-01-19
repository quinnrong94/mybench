#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <mach/thread_act.h>

#define LOOP (1e9)

extern "C" {
    void FmlaLoop12(int);
}

static double get_time(struct timespec *start,
                       struct timespec *end) {
    return end->tv_sec - start->tv_sec + (end->tv_nsec - start->tv_nsec) * 1e-9;
}

void *run(void *arg) {
    int id = *(int*)arg;
    struct timespec start, end;
    double time_used = 0.0;

    // int loop = id < 4 ? LOOP / 2 : LOOP * 2;
    int loop = LOOP;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    FmlaLoop12(loop);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    time_used     = get_time(&start, &end);     // seconds
    int op_floats = 12 * 4 * 2;                 // 12 mla_op (1 mla_op = 4 mul_op + 4 add_op)
    printf("thread %2d, loop %12d: %.6lf GFLOPS\r\n", id, loop, loop * 1.0 * op_floats * 1e-9 / time_used);

    return NULL;
}

void run_without_affinity(int thread_num) {
    printf("\nrun_without_affinity, %d threads...\n", thread_num);
    pthread_t threads[thread_num];
    int ids[thread_num];
    for (int i = 0; i < thread_num; ++i) {
        ids[i] = i;
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_create(&threads[i], NULL, run, &ids[i]);
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
    }
}

void run_with_affinity(int thread_num) {
    printf("\nrun_with_affinity, %d threads...\n", thread_num);
    pthread_t threads[thread_num];
    int ids[thread_num];
    thread_affinity_policy_data_t policyData[thread_num];
    for (int i = 0; i < thread_num; ++i) {
        ids[i] = i;
        policyData[i].affinity_tag = i;
    }

    mach_port_t mach_threads[thread_num];
    for (int i = 0; i < thread_num; ++i) {
        if(pthread_create_suspended_np(&threads[i], NULL, run, &ids[i]) != 0) abort();
        mach_threads[i] = pthread_mach_thread_np(threads[i]);
        thread_policy_set(mach_threads[i], THREAD_AFFINITY_POLICY, (thread_policy_t)&policyData[i], 1);
    }

    for (int i = 0; i < thread_num; ++i) {
        thread_resume(mach_threads[i]);
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char **argv) {
    int affinity = 0;
    if (argc > 1) {
        affinity = atoi(argv[1]);
    }
    int thread_num = 1;
    if (argc > 2) {
        thread_num = atoi(argv[2]);
    }

    if (affinity) {
        run_with_affinity(thread_num);
    } else {
        run_without_affinity(thread_num);
    }

    return 0;
}
