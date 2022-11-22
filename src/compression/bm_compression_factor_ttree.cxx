/**
 * Benchmark the compression factor of TTree-based DAOD_PHYS/LITE files
 * for different compression algorithms.
 *
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <Compression.h>
#include <TError.h>
#include <TFile.h>
#include <TSystem.h>
#include <TTree.h>

#include <filesystem>
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

  TFile *inputFile = TFile::Open(inputPath.c_str());

  if (!inputFile) {
    delete inputFile;
    return 1;
  }

  TTree *xAODCollectionTree = inputFile->Get<TTree>("CollectionTree");

  Int_t compressionAlgorithms[] = {
      ROOT::RCompressionSetting::EAlgorithm::kZLIB, // 1
      ROOT::RCompressionSetting::EAlgorithm::kLZMA, // 2
      ROOT::RCompressionSetting::EAlgorithm::kLZ4,  // 4
      ROOT::RCompressionSetting::EAlgorithm::kZSTD, // 5
  };

  TTree compressionDataTree =
      TTree("compression_factor", "TTree Compression Factors");

  Int_t compAlg;
  Int_t compLvl;
  Float_t compFactor;
  Int_t compSetting;

  compressionDataTree.Branch("compAlg", &compAlg);
  compressionDataTree.Branch("compLvl", &compLvl);
  compressionDataTree.Branch("compFactor", &compFactor);

  const std::string outputDir = "./output/";
  std::filesystem::create_directory(outputDir);

  std::string outputPath;
  TFile *outputFile;

  for (Int_t alg : compressionAlgorithms) {
    for (compLvl = 1; compLvl < 10; ++compLvl) {
      compAlg = alg;
      compSetting = compAlg * 100 + compLvl;
      outputPath = outputDir + "/collection_ttree~" +
                   std::to_string(compSetting) + ".root";

      outputFile = TFile::Open(outputPath.c_str(), "RECREATE", "CollectionTree",
                               compSetting);

      outputFile->WriteObject(xAODCollectionTree, "CollectionTree");

      compFactor = outputFile->GetCompressionFactor();
      compressionDataTree.Fill();

      std::cout << "Total size: \t\t\t" << outputFile->GetSize()
                << "\nCompression algorithm: \t\t"
                << outputFile->GetCompressionSettings()
                << "\nCompression factor: \t\t"
                << outputFile->GetCompressionFactor() << std::endl
                << std::endl;
    }
  }

  std::filesystem::create_directory("./data/");
  TFile *compressionDataFile =
      TFile::Open("./data/ttree_compression_factor.root", "RECREATE");
  compressionDataFile->WriteObject(&compressionDataTree, "compressionDataTree");

  return 0;
}
