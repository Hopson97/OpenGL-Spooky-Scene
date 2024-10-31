#!/bin/bash

target_release() {
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release ../..
    make
    echo "Built target in build/release/"
    cd ../..
}

target_debug() {
    cd debug 
    cmake -DCMAKE_BUILD_TYPE=Debug ../..
    make
    echo "Built target in build/debug/"
    cd ../..
}

# Create folder for distribution
if [ "$1" = "release" ]
then
    if [ -d "$spooky-game" ]
    then
        rm -rf -d spooky-game
    fi

    mkdir -p spooky-game
fi

# Creates the folder for the buildaries
mkdir -p spooky-game 
mkdir -p spooky-game/assets
mkdir -p build
mkdir -p build/release
mkdir -p build/debug
cd build

# Builds target
if [ "$1" = "release" ]
then
    target_release
    cp build/release/spooky-game spooky-game/spooky-game
else
    target_debug
fi

cp -R assets spooky-game/
