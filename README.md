# ATLAS-BM

Collection of benchmarks for ROOT's `TTree` and `RNTuple` in ATLAS `DAOD_PHYS/LITE`.

## Setup

### Docker
Make sure to have Docker (and Docker Compose) installed.
Run:
```sh
docker compose build
```
to build the image.

To run an interactive shell in the container, run
```sh
docker compose run -it --rm atlas-bm /bin/sh
```

To directly execute one of the benchmarks and collect its results instead, run
```sh
docker compose run --rm atlas-bm <EXECUTABLE> <ARGS>
```

For example:
```sh
docker compose run --rm atlas-bm bin/bm_compression_factor_ttree $DAOD_PHYS_FILE
```
(make sure the `$DAOD_PHYS_FILE` env variable is set on the *host*!)

### Local
Run the following commands in the root of this repository (i.e. where this `README` lives):
```sh
cmake -S src/ -B build/
cmake --build build/
```
The executable files can be found in `bin/`.
N.B. the benchmarks themselves are not compiled unless `$AtlasProject` is set.
