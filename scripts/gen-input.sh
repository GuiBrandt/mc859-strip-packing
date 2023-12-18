#!/usr/bin/env bash

N=$1
L=$2
MIN_LENGTH=$3
MAX_LENGTH=$4
MIN_HEIGHT=$5
MAX_HEIGHT=$6
MIN_WEIGHT=$7
MAX_WEIGHT=$8

echo nitems strip_width
echo $N $L

echo item width height weight
for i in $(seq 0 $(($N-1)))
do
    LENGTH=$(shuf -i $MIN_LENGTH-$MAX_LENGTH -n 1)
    HEIGHT=$(shuf -i $MIN_HEIGHT-$MAX_HEIGHT -n 1)
    WEIGHT=$(shuf -i $MIN_WEIGHT-$MAX_WEIGHT -n 1)
    echo $i $LENGTH $HEIGHT $WEIGHT
done
