#! /usr/bin/bash

###
#
# Generate TTree-based AODs with different compression settings using `hadd`.
#
# Author: Florine de Geus (fdegeus@cern.ch)
#

BASE_AOD=$1

if [[ -z $BASE_AOD ]]; then
  echo "Please provide a path to the base (D)AOD."
  exit 1
fi

if [[ ! -e $BASE_AOD ]]; then
  echo "File $BASE_AOD does not exist, exiting..."
  exit 2
fi

COMPRESSION_SETTINGS=(207 404 505)
OUTPUT_DIR="/workdir/data/compressed_ttrees"
LOG_DIR="/workdir/logs"
OUTPUT_FILE_BASE_NAME="DAOD_PHYS"
ORIGINAL_COMPRESSION=$(root $BASE_AOD -q -l -e "int s = _file0->GetCompressionSettings(); cout << s << endl;" 2> $LOG_DIR/hadd.log | tail -n1)

function copy_recompressed() {
  COMPRESSION_SETTING=$1
  OUTPUT_PATH="${OUTPUT_DIR}/${OUTPUT_FILE_BASE_NAME}~${COMPRESSION_SETTING}.root"
  echo "Copying (D)AOD with compression set to $COMPRESSION_SETTING..."
  cmd=("hadd" "-j" "-f${COMPRESSION_SETTING}" "${OUTPUT_PATH}" "${BASE_AOD}")
  "${cmd[@]}" &> $LOG_DIR/hadd.log
  echo "(D)AOD written to ${OUTPUT_PATH}"
}

function copy_original() {
  OUTPUT_PATH="${OUTPUT_DIR}/${OUTPUT_FILE_BASE_NAME}~ORIGINAL.root"
  echo "Copying (D)AOD with original compression settings..."
  cmd=("hadd" "-j" "-f${ORIGINAL_COMPRESSION}" "${OUTPUT_PATH}" "${BASE_AOD}")
  "${cmd[@]}" &> $LOG_DIR/hadd.log
  echo "(D)AOD written to ${OUTPUT_PATH}"
}

function copy_uncompressed() {
  OUTPUT_PATH="${OUTPUT_DIR}/${OUTPUT_FILE_BASE_NAME}~UNCOMPRESSED.root"
  echo "Copying (D)AOD without compression..."
  cmd=("hadd" "-j" "-f0" "${OUTPUT_PATH}" "${BASE_AOD}")
  "${cmd[@]}" &> $LOG_DIR/hadd.log
  echo "(D)AOD written to ${OUTPUT_PATH}"
}

mkdir -p $OUTPUT_DIR
mkdir -p $LOG_DIR
touch $LOG_DIR/hadd.log
sudo chown -R atlas:atlas $LOG_DIR

pids=()

if [[ ! " ${COMPRESSION_SETTINGS[*]} " =~ " ${ORIGINAL_COMPRESSION} " ]]; then
  copy_original &
  pids+=($!)
fi

for c in ${COMPRESSION_SETTINGS[@]}; do
  copy_recompressed $c &
  pids+=($!)
done

copy_uncompressed &
pids+=($!)

wait
exit 0

