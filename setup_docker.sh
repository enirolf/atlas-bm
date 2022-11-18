#! /usr/bin/sh

source /release_setup.sh

cmake -S src -B build
cmake --build build

exec "$@"
