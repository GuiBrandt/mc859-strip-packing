#!/usr/bin/bash

for N in 10 15 20
do
for I in 1 2 3
do
for INSTANCE in instances/mc859-level-strip-packing_${I}_${N}_*
do
    EXT=${INSTANCE##*.}

    INSTANCE_NAME=$(basename $INSTANCE .$EXT)
    echo Running $INSTANCE_NAME

    OUTPUT_DIR=output/flow/$INSTANCE_NAME
    mkdir -p $OUTPUT_DIR

    time build/mc859-strip-packing-flow -o $OUTPUT_DIR $INSTANCE > $OUTPUT_DIR/stdout.txt
    echo
    echo Best: $(grep -Po "Cost: \K.*" $OUTPUT_DIR/exact.txt)
    echo
done
done
done