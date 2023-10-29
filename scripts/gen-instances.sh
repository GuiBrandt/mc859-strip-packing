#!/usr/bin/sh

mkdir -p instances/random

for size in 50 500 1000
do
for maxlen in 1 10 30
do
for maxheight in 1 10 100
do
for maxweight in 1 100
do
    FILENAME=instances/random/random-$size-$maxlen-$maxheight-$maxweight.yml
    build/mc859-strip-packing-gen-instances \
        --seed 1729 $size 100 1 $maxlen 1 $maxheight 1 $maxweight \
        > $FILENAME
    echo Generated $FILENAME
done
done
done
done
