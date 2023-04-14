#!/usr/bin/env bash

function bm_size() {
  storage_name=$1
  storage_type=$2

  for phys_file_type in {data,mc}; do
    results_file=results/size_${phys_file_type}.txt

    for compression in {0,201,207,505}; do
      source_file=data/daod_phys_benchmark_files/${phys_file_type}/DAOD_PHYS.${storage_type}.root~${compression}

      echo "Running for $storage_type ($phys_file_type, $compression)..."

      results=$(./bin/bm_size -i $source_file -n $storage_name -s $storage_type)
      echo "$results"
      echo "$results" >> $results_file
    done
  done
}

function main() {
  bm_size CollectionTree ttree
  bm_size RNT:CollectionTree rntuple
}

main
