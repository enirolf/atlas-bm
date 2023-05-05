#!/usr/bin/env bash

COLD_CACHE=true

function drop_caches() {
  sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
}

function bm_readspeed() {
  storage_name=$1
  storage_type=$2
  results_dir=$3/$storage_type

  if [ "$(root-config --has-uring)" = "yes" ]; then
    results_dir=${results_dir}_uring
  fi

  mkdir -p $results_dir

  for phys_file_type in {data,mc}; do
    for compression in {0,201,207,505}; do
      source_file=${SOURCE_DIR}/${phys_file_type}/DAOD_PHYS.${storage_type}.root~${compression}
      results_file=${results_dir}/readspeed_${phys_file_type}_${compression}.txt

      echo "Running for $storage_type ($phys_file_type, $compression)..."

      for (( i = 0; i < $N_REPETITIONS; i++ )); do
        if [ "$COLD_CACHE" = true ]; then
          drop_caches
        fi

        if [ "$COLD_CACHE" = false ] && [ "$i" = 0 ]; then
          continue
        fi

        results=$(bin/bm_readspeed -i $source_file -n $storage_name -s $storage_type 2>&1)
        echo "$results" >> $results_file
      done
    done
  done
}

function main() {
  bm_readspeed CollectionTree ttree $1
  bm_readspeed RNT:CollectionTree rntuple $1
}


SOURCE_DIR=$1
if [ -z "$SOURCE_DIR" ]; then
  echo "Please specify the source directory of the benchmark data"
  exit 1
fi

# Get the number of repetitions from the command line or use the default value (10)
N_REPETITIONS=${2:-10}

main ${3:-results/}
