#!/usr/bin/env bash
for I in $(seq 1 3)
do
for N in 10 15 20
do
scripts/gen-input.sh $N 100 1 100 1 100 1 5 > instances/mc859-level-strip-packing_${I}_${N}_100_1_100_1_100_1_5
done
done
