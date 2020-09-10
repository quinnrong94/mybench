#include "lat_mem.h"

int TEMP;

static int ITER = 10000;

static int get_iteration() {
    return ITER;
}

void do_prepare(lat_mem_state_t *state) {

    size_t buflen = 1024*1024*256;
    state->buf = (char *)malloc(buflen);
    if(!state->buf) {
        perror("malloc buf error");
        exit(1);
    }
    memset(state->buf, 0, buflen);

    char *buf = state->buf;
    size_t stride = state->stride;
    int i = 0;
    for(i = stride; i < state->buflen; i += stride) {
        *(char **)&buf[i-stride] = (char *)&buf[i];
    }
    *(char **)&buf[i-stride] = (char *)&buf[0];
}

void do_cooldown(lat_mem_state_t *state) {
    if(state->buf)
        free(state->buf);
}

#define	ONE	p = (char **)*p;
#define	FIVE	ONE ONE ONE ONE ONE
#define	TEN	FIVE FIVE
#define	FIFTY	TEN TEN TEN TEN TEN
#define	HUNDRED	FIFTY FIFTY

void do_latency(unsigned long iter, lat_mem_state_t *state) {
    char **p = (char **)&state->buf[0];
    while(iter --) {
        HUNDRED	
    }
    TEMP = (long)p;
}

double get_cache_latency(lat_mem_state_t *state) {
    struct timespec st, ed;
    unsigned long elapse;
    int loopcnt, loop = 30;
    int index = loop >> 2;
    int iter, iteration = get_iteration();

    result_t *r = (result_t *)malloc(sizeof(result_t) * loop);
    loopcnt = loop;

    while(loopcnt --) {

        if(state->prepare)
            state->prepare(state);

        clock_gettime(CLOCK_REALTIME, &st);
        state->bench(iteration, state);
        clock_gettime(CLOCK_REALTIME, &ed);
        elapse = get_total_ns(&st, &ed);
        save_result(r, loopcnt, elapse, iteration);

        if(state->cooldown)
            state->cooldown(state);
    }

    qsort(r, loop, sizeof(result_t ), cmp_result);
    double latency = get_latency(r[index].elapse / 100, r[index].iter); // convert us to ns
    return latency;
}

size_t l1_step(size_t k) {

    if (k < 16*1024) {
        k = k * 4;
    } else if (k < 64*1024) {
        k += 16*1024;
    } else {
        size_t s;
        for (s = 128 * 4096; s <= k; s *= 2)
            ;
        k += s / 4;
    }
    return (k);
}

void bench_l1_latency() {
    lat_mem_state_t state;
    memset(&state, 0, sizeof(lat_mem_state_t));
    state.prepare = do_prepare;
    state.bench = do_latency;
    state.cooldown = do_cooldown;
    size_t strides[] = {64, 1024, 4096, 8192};
    size_t buflen = 1024*1024; // 1024KB

    printf("\tBenchmark of Latency of L1 Cache\n\n");
    for(int i=0; i < sizeof(strides)/sizeof(size_t); i++) {
        size_t stride = strides[i];
        for(size_t len = 1024; len <= buflen; len = l1_step(len)) {
            if(stride < len) {
                state.buflen = len;
                state.stride = stride;
                printf("\tbuflen = %5luKB\tstride =  %4lu\tL1 Cache Latency: %lfns\n", (unsigned long)(len >> 10),
                       (unsigned long)stride, get_cache_latency(&state));
            }
        }
        printf("\n");
    }
}

size_t l2_step(size_t k) {

    if (k < 128*1024) {
        k = k * 2;
    } else if (k < 256*1024) {
        k += 32*1024;
    } else {
        size_t s;
        for (s = 512 * 4096; s <= k; s *= 2)
            ;
        k += s / 4;
    }
    return (k);
}

void bench_l2_latency() {
    lat_mem_state_t state;
    memset(&state, 0, sizeof(lat_mem_state_t));
    state.prepare = do_prepare;
    state.bench = do_latency;
    state.cooldown = do_cooldown;
    size_t strides[] = {4096, 8192, 16384};
    size_t buflen = 8*1024*1024; // 1024KB

    printf("\tBenchmark of Latency of L2 Cache\n\n");
    for(int i=0; i < sizeof(strides)/sizeof(size_t); i++) {
        size_t stride = strides[i];
        for(size_t len = 32*1024; len <= buflen; len = l2_step(len)) {
            if(stride < len) {
                state.buflen = len;
                state.stride = stride;
                printf("\tbuflen = %5luKB\tstride =  %4lu\tL1 Cache Latency: %lfns\n", (unsigned long)(len >> 10),
                       (unsigned long)stride, get_cache_latency(&state));
            }
        }
        printf("\n");
    }
}

size_t l3_step(size_t k) {

    if (k < 128*1024) {
        k = k * 2;
    } else if (k < 256*1024) {
        k += 32*1024;
    } else {
        size_t s;
        for (s = 512 * 4096; s <= k; s *= 2)
            ;
        k += s / 8;
    }
    return (k);
}

void bench_l3_latency() {
    lat_mem_state_t state;
    memset(&state, 0, sizeof(lat_mem_state_t));
    state.prepare = do_prepare;
    state.bench = do_latency;
    state.cooldown = do_cooldown;
    size_t strides[] = {4096, 8192, 16384};

    printf("\tBenchmark of Latency of L3 Cache\n\n");

    size_t buflen = 32*1024*1024; // 1024KB
    for(int i=0; i < sizeof(strides)/sizeof(size_t); i++) {
        size_t stride = strides[i];
        for(size_t len = 32*1024; len <= buflen; len = l3_step(len)) {
            if(stride < len) {
                state.buflen = len;
                state.stride = stride;
                printf("\tbuflen = %6luKB\tstride =  %6lu\tL1 Cache Latency: %lfns\n", (unsigned long)(len >> 10),
                       (unsigned long)stride, get_cache_latency(&state));
            }
        }
        printf("\n");
    }
}

void bench_cache_latency() {
    printf("\nbenchmark on Cache Latency\n");
    printf("-------------------------------------\n");

    bench_l1_latency();
    bench_l2_latency();
    bench_l3_latency();

    printf("\n--------------end--------------------\n");
}
