#! /usr/bin/sh

source /release_setup.sh

mkdir build

sudo chown -R atlas output/ data/

cmake -S src -B build
cmake --build build

exec $@
