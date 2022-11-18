/**
 * Benchmark the compression rate and speed of TTree-based DAOD_PHYS/LITE files
 * for different compression algorithms.
 *
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <TError.h>
#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <string>

#include <unistd.h>

int main(int argc, char **argv) {
  // We ignore dictionary warnings (for now...)
  gErrorIgnoreLevel = kError;

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
            << "\nCompression algorithm: \t\t\t"
            << inputFile->GetCompressionAlgorithm()
            << "\nCompression factor: \t\t\t"
            << inputFile->GetCompressionFactor() << std::endl;

  TTree *collectionTree = inputFile->Get<TTree>("CollectionTree");

  std::cout << "\nCollectionTree total size (in bytes): \t"
            << collectionTree->GetTotBytes()
            << "\nCollectionTree zip size (in bytes): \t"
            << collectionTree->GetZipBytes() << std::endl;

  TFile *outputFile0 = TFile::Open("data/collection_ttree~0.root", "RECREATE",
                                   "CollectionTree", 0);
  outputFile0->WriteObject(collectionTree, "CollectionTree");

  std::cout << "Total size: \t\t\t" << outputFile0->GetSize()
            << "\nCompression algorithm: \t\t"
            << outputFile0->GetCompressionAlgorithm()
            << "\nCompression factor: \t\t"
            << outputFile0->GetCompressionFactor() << std::endl;

  TFile *outputFile505 = TFile::Open("data/collection_ttree~505.root",
                                     "RECREATE", "CollectionTree", 505);
  outputFile505->WriteObject(collectionTree, "CollectionTree");

  std::cout << "Total size: \t\t\t" << outputFile505->GetSize()
            << "\nCompression algorithm: \t\t"
            << outputFile505->GetCompressionAlgorithm()
            << "\nCompression factor: \t\t"
            << outputFile505->GetCompressionFactor() << std::endl;

  return 0;
}
