# ATLAS-BM

Collection of benchmarks for ROOT's **TTree** and **RNTuple** in ATLAS **DAOD_PHYS**, using **RDataFrame**.

## General project structure

Each type of benchmark has a separate directory with all the relevant code: `bm-size` and `bm-readspeed`. Additionally, the `bm-utils` directory provides some common functions and scripts. For each benchmark, a driver script is provided in the root of this repo.

## Building the benchmarks

***IMPORTANT!*** To get access to the raw I/O throughput for RNTuple, a patched version of ROOT is required that enables RNTuple metrics in the RDataFrame initialization method. This patch can be found here: https://github.com/enirolf/root/releases/tag/chep23. Depending on how recent the benchmark input files are, a rebase with ROOT's upstream master may be required.

The benchmark code can be built with CMake. Build the project by running:
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
The data used for the benchmarks are DAOD_PHYS data and MC samples, from the following sources:

### Data
`data18_13TeV.00348885.physics_Main.deriv.DAOD_PHYS.r13286_p4910_p5440`, 209062 events concatenated from the following samples:
* `DAOD_PHYS.31140487._000036.pool.root.1`
* `DAOD_PHYS.31140487._000037.pool.root.1`
* `DAOD_PHYS.31140487._000038.pool.root.1`

### MC
`mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13167_p5169` data set, 180000 events from the sample `DAOD_PHYS.29445526._000036.pool.root`.

In general, other input can also be used as long as they contain the `AuxDyn.(pt|eta|phi|m)` branches for the following containers:
* `Electrons`
* `Photons`
* `TauJets`
* `TauJets_MuonRM`
* `DiTauJets`
* `DiTauJetsLowPt`
* `TauNeutralParticleFlowObjects`
* `TauNeutralParticleFlowObjects_MuonRM`

The ready-to-use input data set used for CHEP 2023 (with both TTree and RNTuple input files) is available upon request. To generate them from scratch, make sure you have access to the xAOD dictionaries (e.g. through an Athena release on LXPLUS, with the same ROOT version you intend to use for running the benchmarks) and run `bm-utils/write_trees.sh` (as a shell script) and `bm-utils/write_ntuples.C` (as a ROOT macro). They must be run in this order! Make sure the file paths defined at the top of each macro are correct before running them, or adjust accordingly. Depending on your machine and the size of the source file, executing these macros might take a while.

## Running the benchmarks

```sh
./bin/bm_readspeed (-h|-i INPUT_PATH -s (ttree|rntuple) [-n STORE_NAME])
./bin/bm_size (-h|-i INPUT_PATH -s (ttree|rntuple) [-n STORE_NAME])
```
`STORE_NAME` by default is `CollectionTree`.

To automate running the benchmarks for different configurations (compression settings, storage media, etc.) and repeat each run multiple times, use the `bm_readspeed.sh` and `bm_size.sh` scripts:
```sh
./bm_readspeed.sh $INPUT_DIR $N_RUNS $RESULTS_DIR
./bm_size.sh $RESULTS_DIR
```
Where `$INPUT_DIR` is the path to the directory containing all DAOD_PHYS samples (default is `./data`), `$N_RUNS` (default is 10) is the number of run repetitions and `$RESULTS_DIR` is the result output directory (default is `./results` but you might should it for a specific storage medium, e.g. `./results/ssd` to be able to plot the results for readspeed).

## Plotting the results

### Readspeed

Each benchmark directory contains a ROOT macro for producing plots. They assume results are stored in a `results` with a subdirectory for the following storage media:
* `sdd`
* `hdd`
* `tmpfs`
* `xrootd`

There are two plotting macros: `plot_overview_per_medium.C` plots for a given input sample the read throughput (both in wall time and raw I/O) for each medium for both TTree and RNTuple given the different compression settings. `plot_throughput_ratio.C` plots the ratio of read throughput between TTree and RNTuple for each storage medium (per compression setting).

### Size

There is one plotting macro `plot_size.C`, which plots the size of a given compression setting for both TTree and RNTuple, along with the ratio between the two.
