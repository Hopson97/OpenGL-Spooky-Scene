#!/bin/bash

if [ "$1" = "release" ]
then
    ./build/release/spooky-game
else
    ./build/debug/spooky-game
fi