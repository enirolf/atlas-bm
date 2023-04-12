#!/usr/bin/env bash

function drop_caches() {
  echo 3 > /proc/sys/vm/drop_caches
}

function bm_cold_cache() {
  storage_name=$1
  storage_type=$2
  for phys_file_type in {data,mc}; do
    for compression in {0,201,207,505}; do
      source_file=$SOURCE_DIR/${phys_file_type}/DAOD_PHYS.${storage_type}.root~${compression}
      results_file=results/readspeed_cold_${storage_type}_${phys_file_type}_${compression}.txt

      echo "Running for $storage_type ($phys_file_type, $compression)..."

      for (( i = 0; i < $N_REPETITIONS; i++ )); do
        results=$(bin/bm_readspeed -i $source_file -n $storage_name -m $storage_type)
        echo "$results"
        echo "$results" >> $results_file
        drop_caches
      done
    done
  done
}

function main() {
  if [ "$EUID" -ne 0 ]; then
    echo "Please run with sudo (required to clear caches between runs)"
    exit 1
  fi

  mkdir -p results

  bm_cold_cache CollectionTree ttree
  bm_cold_cache RNT:CollectionTree rntuple
}

SOURCE_DIR=$1
if [ -z "$SOURCE_DIR" ]; then
  echo "Please specify the source directory of the benchmark data"
  exit 1
fi

# Get the number of repetitions from the command line or use the default value (10)
N_REPETITIONS=${2:-10}

main
