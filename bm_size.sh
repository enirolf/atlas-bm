#!/usr/bin/env bash

function bm_size() {
  storage_type=$1
  results_dir=$2

  for phys_file_type in {data,mc}; do
    results_file=${results_dir}/size_${phys_file_type}.data

    for compression in {0,201,207,404,505}; do
      source_file=data/daod_phys_benchmark_files/${phys_file_type}/DAOD_PHYS.${storage_type}.root~${compression}

      echo "Running for $storage_type ($phys_file_type, $compression)..."

      results=$(./bin/bm_size -i $source_file -s $storage_type)
      echo "$results"
      echo "$results" >> $results_file
    done
  done
}

function main() {
  bm_size ttree ${1:-results/}
  bm_size rntuple ${1:-results/}
}

main
