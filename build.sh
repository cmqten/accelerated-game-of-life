#!/usr/bin/env bash

PROJDIR=`dirname $(realpath $0)`
OUTDIR="$PROJDIR/build"
NUMPROCS=`cat /proc/cpuinfo | grep "^processor" | wc -l`

function usage_msg {
    echo "Usage:"
    echo "    build.sh [options]"
    echo ""
    echo "Options:"
    echo "    -c        : clean build tree instead of compiling"
    echo "    -j N      : use N jobs to compile"
}

while getopts "cj:" arg; do 
    case $arg in
        c)  rm -rf $OUTDIR; exit 0;; # clean

        j)  # number of parallel jobs to spawn during compile
            if [ ${OPTARG} -eq ${OPTARG} 2> /dev/null ]; then
                NUMPROCS=${OPTARG}
            else
                usage_msg; exit 1;
            fi;;

        ?)  usage_msg; exit 1;;
    esac
done

if [ -e $OUTDIR ]; then # Build if build directory exists already
    cd $OUTDIR
    make -j $NUMPROCS
elif [ ! -e $OUTDIR ]; then # Create build directory first
    read -e -p "Enter C++ compiler [default: detected by CMake]: " CXX
    export CXX
    read -e -p "Enter OpenCL directory [default: none]: " OCLDIR
    export OCLDIR

    mkdir $OUTDIR
    cd $OUTDIR
    cmake ../
    make -j $NUMPROCS
else
    usage_msg
fi