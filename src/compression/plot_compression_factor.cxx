/**
 * Plot the compression factor and speed of TTree-based DAOD_PHYS/LITE files
 * for different compression algorithms.
 *
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <ROOT/RDataFrame.hxx>

#include <iostream>

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cerr << "No file specified, exiting..." << std::endl;
    return 1;
  }

  std::string inputPath = argv[1];

  ROOT::RDataFrame compressionFactorDF =
      ROOT::RDataFrame("compression_factor", inputPath);
  return 0;
}
