#!/usr/bin/env bash

BUILD_DIR="./build"

if [ -e $BUILD_DIR ]; then
    cd $BUILD_DIR
    make -j 4
    exit
fi

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ../
make -j 4