#!/bin/bash

function run() {
    ./bench $1
}

for thread_num in 1 2 4 8 16
do
    run ${thread_num} | tee -a log.txt
done
