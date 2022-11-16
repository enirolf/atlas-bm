/**
 * Benchmark the compression rate and speed of TTree-based DAOD_PHYS/LITE files
 * for different compression algorithms.
 *
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <TFile.h>
#include <TTree.h>

#include <iostream>

#include <unistd.h>

int main(int argc, char **argv)
{
  std::string inputPath = "";

  int c;

  while((c = getopt(argc, argv, "i")) != -1) {
    switch(c) {
    case 'i':
      inputPath = optarg;
      break;
    default:
      std::cerr << "Unknown option -" << c << std::endl;
      return 1;
    }
  }

  std::cout << "Hello, world!" << std::endl;
  // TFile &iputFile(TFile::Open(inputPath));

  return 0;
}
