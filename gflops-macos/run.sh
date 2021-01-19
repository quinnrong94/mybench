#!/bin/bash

function run() {
    ./bench $1 $2
}

for affinity in 0 1
do
    for thread_num in 1 2 4 8
    do
        run ${affinity} ${thread_num}
    done
done
