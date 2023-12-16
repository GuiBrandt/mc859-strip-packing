#!/usr/bin/sh

mkdir -p instances/random

for i in 1 2 3 4 5
do
for size in 5 10 15 20
do
for maxlen in 100
do
for maxheight in 100
do
    FILENAME=instances/random/random-$i-$size-$maxlen-$maxheight.yml
    build/mc859-strip-packing-gen-instances \
        --seed "${i}729" $size 100 1 $maxlen 1 $maxheight 1 5 \
        > $FILENAME
    echo Generated $FILENAME
done
done
done
done
