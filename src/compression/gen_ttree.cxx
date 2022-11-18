/**
 * Benchmark the compression rate and speed of TTree-based DAOD_PHYS/LITE files
 * for different compression algorithms.
 *
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <string>

#include <unistd.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "No file specified, exiting..." << std::endl;
    return 1;
  }

  std::string inputPath = argv[1];

  std::cout << inputPath << std::endl;
  TFile *inputFile = TFile::Open(inputPath.c_str());

  if (!inputFile) {
    delete inputFile;
    return 1;
  }

  std::cout << "Total xAOD size (in bytes): \t\t" << inputFile->GetSize()
            << std::endl;

  TTree *collectionTree = inputFile->Get<TTree>("CollectionTree");

  std::cout << "CollectionTree total size (in bytes): \t"
            << collectionTree->GetTotBytes() << std::endl;

  std::cout << "CollectionTree zip size (in bytes): \t"
            << collectionTree->GetZipBytes() << std::endl;

  TFile *outputFile = TFile::Open("data/collection_ttree~0.root", "RECREATE",
                                  "CollectionTree", 0);
  outputFile->WriteObject(collectionTree, "CollectionTree");

  return 0;
}
