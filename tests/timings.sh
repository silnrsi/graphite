#!/bin/bash

if test $# = 0;
then
    echo $0 font textfile
else
    echo $1
    echo $2
    echo No cache
    tests/comparerenderer/comparerenderer -s 12 -f $1 -t $2 -n
    for cache in 500 1000 5000 10000 20000 50000;
    do
        echo Cache=$cache
        tests/comparerenderer/comparerenderer -s 12 -f $1 -t $2 -n --seg-cache $cache
    done
fi

