# ATLAS-BM

Collection of benchmarks for ROOT's `TTree` and `RNTuple` in ATLAS `DAOD_PHYS/LITE`.

## Compression

### Generating DAODs with different compression settings

This must be done in an ATLAS Analysis release, e.g. on lxplus or with Docker.

N.B. The `AnalysisBase` release doesn't have access to all required dictionaries and therefore won't work! Use `AthAnalysis`.

To use docker, run the following command to mount the current directory, enable X11 forwarding and open an interactive shell:
```sh
docker run --net host -i -t -v $(pwd):/workdir -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY atlas/athanalysis
```
Make sure to run `source /release_setup.sh` each time you start the container.

To get read/write access to the `data` directory from within the container, run
```sh
sudo chown -R atlas:atlas data
```
in the container (change it back to the host user when you stop working in the container).

To create the differently compressed versions of a given (D)AOD, run
```sh
cd scripts
./recompress_daod <DAOD_PATH>
```
