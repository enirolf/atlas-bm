# ATLAS-BM

Collection of benchmarks for ROOT's `TTree` and `RNTuple` in ATLAS `DAOD_PHYS/LITE`.

## Environment setup

All benchmarks must be done in an ATLAS Analysis release, e.g. on lxplus or with Docker (using the Athena VSCode devcontainer is highly recommended!).

Since we require ROOT7, we need to use a nightly (or in some cases, a local build -- this setup is not yet documented here):

```
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/AtlasSetup/current/AtlasSetup/scripts/asetup.sh master--dev3LCG,latest,Athena
```

To use docker (w/o VSCode devcontainers), run the following command to mount the current directory and CVMFS, enable X11 forwarding and open an interactive shell:
```sh
docker run --net host -i -t -v $(pwd):/workdir:rw -v /cvmfs:/cvmfs -v /tmp/.X11-unix:/tmp/.X11-unix -v /afs:/afs -e DISPLAY=$DISPLAY --name atlas atlas/centos7-atlasos-dev:latest-gcc11
```

## General project structure

This repository contains two main directories of interest for running benchmarks and analysing their results: `macros`, which contains the necessary ROOT macros for preparing the files as well as analysing and plotting the benchmark results and `benchmarks/`, which runs the benchmarks and saves their results. These results are by default saved in the `results/` directory (which will be created if it does not yet exist).

### Building the benchmarks

The benchmark code can be built with CMake. Build the project here by running:
```sh
cmake .
cmake --build .
```
or alternatively by creating a separate build directory:
```sh
mkdir build
cmake -S . -B build
cmake --build build
```
The compiled binaries can be found in `bin/`.

## Benchmarks
### Size and compression

#### Generating DAODs with different compression settings
There are two macros to achieve this: `write_trees.C` and `write_ntuples.C`. They must be run in this order! Make sure the file paths defined at the top of each macro are correct before running them, or adjust accordingly. Depending on your machine and the size of the source file, executing these macros might take a while (feel free to get a coffee).

#### Running the benchmarks

Build the benchmarks using the instructions described above and run
```sh
./bin/bm_size
```

#### Analysing the benchmarks
Run
```sh
root plot_size.C
```
