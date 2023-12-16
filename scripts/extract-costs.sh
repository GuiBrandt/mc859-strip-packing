#!/usr/bin/bash

echo "instance & first-fit & best-fit & brkga \\\\"

for INSTANCE in output/*
do
    INSTANCE_NAME=$(basename $INSTANCE)

    FIRST_FIT_COST=$(grep -Po "Cost: \K.*" $INSTANCE/first-fit.txt)
    BEST_FIT_COST=$(grep -Po "Cost: \K.*" $INSTANCE/best-fit.txt)
    BRKGA_COST=$(grep -Po "Cost: \K.*" $INSTANCE/brkga.txt)

    if (($(echo $BRKGA_COST != 0 | bc -l)))
    then
        FIRST_FIT_COST=$(echo $FIRST_FIT_COST / $BRKGA_COST | bc -l)
        BEST_FIT_COST=$(echo $BEST_FIT_COST / $BRKGA_COST | bc -l)
    else
        if (($(echo $FIRST_FIT_COST == 0 | bc -l)))
        then
            FIRST_FIT_COST=1.0
        else
            FIRST_FIT_COST=INF
        fi
        if (($(echo $BEST_FIT_COST == 0 | bc -l)))
        then
            BEST_FIT_COST=1.0
        else
            BEST_FIT_COST=INF
        fi
    fi

    echo "$INSTANCE_NAME & $FIRST_FIT_COST & $BEST_FIT_COST & 1.0 \\\\"
done
