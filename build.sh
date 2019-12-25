#!/usr/bin/env bash

PROJDIR=`dirname $(realpath $0)`
OUTDIR="$PROJDIR/build"
NUMPROCS=`cat /proc/cpuinfo | grep "^processor" | wc -l`

if [ -e $OUTDIR ]; then # Build if build directory exists already
    cd $OUTDIR
    make -j $NUMPROCS
else # Create build directory first
    mkdir $OUTDIR
    cd $OUTDIR
    cmake ../
    make -j $NUMPROCS
fi