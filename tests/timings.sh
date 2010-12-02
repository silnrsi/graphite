#!/bin/bash

if test $# = 0;
then
    echo $0 font textfile
else
    echo $1
    if ! test -f $1 || test `basename $1 .ttf` == `basename $1`;
    then echo Font $1 does not exist or does not have a ttf extension;
    fi
    echo $2
    if ! test -f $2;
    then echo Text $2 does not exist;
    fi
    echo No cache
    tests/comparerenderer/comparerenderer -s 12 -f $1 -t $2 -n
    for cache in 500 1000 5000 10000 20000 50000;
    do
        echo Cache=$cache
        tests/comparerenderer/comparerenderer -s 12 -f $1 -t $2 -n --seg-cache $cache
    done
fi

