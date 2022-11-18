# ATLAS-BM

Collection of benchmarks for ROOT's `TTree` and `RNTuple` in ATLAS `DAOD_PHYS/LITE`.

## Setup

### Docker
Make sure to have Docker (and Docker Compose) installed.
Run:
```sh
docker compose build
docker compose run --rm atlas-bm /bin/sh
```
To build the project and start an interactive shell in the container.

### Local
Run the following commands in the root of this repository (i.e. where this `README` lives):
```sh
cmake -S src/ -B build/
cmake --build build/
```
The executable files can be found in `bin/`.
