# ATLAS-BM

Collection of benchmarks for ROOT's `TTree` and `RNTuple` in ATLAS `DAOD_PHYS(LITE)`.

## Environment setup

Currently, the benchmarks are ROOT-only and therefore do not require an ATLAS release. However, to prepare the data files, a release is required since we need access to the dictionaries. The recommended way to do so is with a VSCode devcontainer, for which a configuration file is provided.

Since we require ROOT7, we need to use a nightly LCG build:

```
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/AtlasSetup/current/AtlasSetup/scripts/asetup.sh master--dev3LCG,latest,Athena
```

Additionally, a patch to ROOT is required to take of dots in branch names which are currently not required in RNTuple. This patch can be found [here](https://github.com/enirolf/root/tree/athena-patches). TODO: document steps to build.

## General project structure

Each type of benchmark has a separate directory with all the relevant code: `bm-size` and `bm-readspeed`. Additionally, the `bm-utils` directory provides some common functions and scripts. For each benchmark, a driver script is provided in the root of this repo.

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

## Benchmark DAODs
Pregenerated DAODs can be found [here](https://cernbox.cern.ch/s/jRZWjRjSRJBjQKg). To generete them from scratch, run `bm-utils/write_trees.sh` (as a shell script) and `bm-utils/write_ntuples.C` (with ROOT). They must be run in this order! Make sure the file paths defined at the top of each macro are correct before running them, or adjust accordingly. Depending on your machine and the size of the source file, executing these macros might take a while, so don't hestitate to go grab a coffee.

### Plotting the results

Each benchmark directory contains a ROOT macro for producing plots. They need some manual tweaking of result paths etc., this is a work in progress.
