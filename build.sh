#!/usr/bin/env bash

PROJDIR=`dirname $(realpath $0)`
OUTDIR="$PROJDIR/build"

function usage_msg {
    echo "Usage:"
    echo "    build.sh : build project"
    echo "    build.sh clean : clean project"
}

if [ $# == 0 ] && [ -e $OUTDIR ]; then # Build if build directory exists already
    cd $OUTDIR
    make -j 4
elif [ $# == 0 ] && [ ! -e $OUTDIR ]; then # Create build directory first
    mkdir $OUTDIR
    cd $OUTDIR
    cmake ../
    make -j 4
elif [ $# == 1 ]; then 
    if [ $1 == "clean" ]; then # Delete build directory
        rm -rf $OUTDIR
    else
        usage_msg
    fi    
else
    usage_msg
fi