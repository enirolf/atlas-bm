#! /usr/bin/env bash

function write_mc_trees() {
    source_file=$1
    target_file_base=$2
    # for compression in {0,201,207,505}; do
        cmd="hadd -f404 -O ${target_file_base}~404 ${source_file}"
        echo $cmd
        eval $cmd
    # done
}

function write_data_trees() {
    source_file=$1
    target_file_base=$2
    # for compression in {0,201,207,505}; do
        # cmd="hadd -f${compression} -O ${target_file_base}~${compression} ${source_file}"
        cmd="hadd -f404 -O ${target_file_base}~404 ${source_file}"
        echo $cmd
        eval $cmd
    # done
}

write_mc_trees \
    "data/daod_phys_benchmark_files/sources/mc20_13TeV/*" \
    "data/daod_phys_benchmark_files/mc/DAOD_PHYS.ttree.root"

write_data_trees \
    "data/daod_phys_benchmark_files/sources/data18_13TeV/*" \
    "data/daod_phys_benchmark_files/data/DAOD_PHYS.ttree.root"
