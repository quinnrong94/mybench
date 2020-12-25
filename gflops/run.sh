#!/system/bin/sh

taskset -a  1 ./test64
taskset -a  2 ./test64
taskset -a  4 ./test64
taskset -a  8 ./test64
taskset -a 10 ./test64
taskset -a 20 ./test64
taskset -a 40 ./test64
taskset -a 80 ./test64
