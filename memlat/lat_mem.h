#ifndef _LAT_MEM_H
#define _LAT_MEM_H

#include "bench.h"

typedef struct lat_mem_state {
    bench_f prepare;
    bench_f bench;
    bench_f cooldown;
    char *buf;
    size_t buflen;
    size_t min_stride;
    size_t max_stride;
    size_t stride;
} lat_mem_state_t;

#endif
