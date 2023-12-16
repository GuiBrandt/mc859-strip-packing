#!/usr/bin/bash

for INSTANCE in instances/**/*.yml
do
    EXT=${INSTANCE##*.}

    INSTANCE_NAME=$(basename $INSTANCE .$EXT)
    echo Running $INSTANCE_NAME

    OUTPUT_DIR=output/$INSTANCE_NAME
    mkdir -p $OUTPUT_DIR

    time build/mc859-strip-packing-heuristics -o $OUTPUT_DIR $INSTANCE > $OUTPUT_DIR/stdout.txt
    echo
    echo First Fit: $(grep -Po "Cost: \K.*" $OUTPUT_DIR/first-fit.txt)
    echo Best Fit: $(grep -Po "Cost: \K.*" $OUTPUT_DIR/best-fit.txt)
    echo BRKGA: $(grep -Po "Cost: \K.*" $OUTPUT_DIR/brkga.txt)
    echo
done
