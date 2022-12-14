/**
 * Benchmark the compression factor of TTree-based DAOD_PHYS/LITE files
 * for different compression algorithms.
 *
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <TApplication.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TError.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TLatex.h>
#include <TLine.h>
#include <TMath.h>
#include <TStyle.h>
#include <TSystem.h>

#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <unistd.h>

#include "util.hxx"

// #include "bm_compression_factor.hxx"

namespace fs = std::filesystem;

int ttreeTypeCompression(const fs::path treeDir = "./data/compressed_ttrees/",
                         const fs::path resultsDir = "./results/compression/") {
  TFile *treeFile;
  TTree *collectionTree;

  fs::create_directories(resultsDir);
  fs::path resultsPath;
  std::fstream resultsFile;

  std::map<int, std::string> compressionAlgoMap;
  mkCompAlgoMap(&compressionAlgoMap);

  int compressionSetting;
  std::string className;

  for (auto &f : fs::directory_iterator(treeDir)) {
    std::string filePath = f.path();

    treeFile = TFile::Open(filePath.c_str(), "READ");
    collectionTree = treeFile->Get<TTree>("CollectionTree");

    compressionSetting = treeFile->GetCompressionSettings();
    std::cout << compressionSetting << " "
              << compressionAlgoMap[compressionSetting] << std::endl;
    resultsPath =
        resultsDir / Form("results_type_compression~ttree-%s.txt",
                          compressionAlgoMap[compressionSetting].c_str());

    resultsFile.open(resultsPath, std::ios_base::out);
    if (!resultsFile.is_open()) {
      std::cerr << "Failed to open " << resultsPath << std::endl;
      return 1;
    }

    for (auto br :
         TRangeDynCast<TBranch>(collectionTree->GetListOfBranches())) {
      if (!br || std::strcmp(br->GetClassName(), "") == 0) {
        continue;
      }

      className = br->GetClassName();
      removeSpaces(&className);
      resultsFile << className << " " << br->GetTotBytes() << "\n";
    }

    resultsFile << std::endl;
    resultsFile.close();
  }

  return 0;
}

int ttreeFileCompression(const fs::path treeDir = "./data/compressed_ttrees/",
                         const fs::path resultsDir = "./results/compression/") {
  TFile *treeFile;
  TTree *collectionTree;

  fs::create_directories(resultsDir);
  fs::path resultsPath = resultsDir / "results_file_compression~ttree.txt";
  std::fstream resultsFile;
  resultsFile.open(resultsPath, std::ios_base::out);

  if (!resultsFile.is_open()) {
    std::cerr << "Failed to open " << resultsPath << std::endl;
    return 1;
  }

  for (auto &f : fs::directory_iterator(treeDir)) {
    std::string filePath = f.path();

    treeFile = TFile::Open(filePath.c_str(), "READ");
    collectionTree = treeFile->Get<TTree>("CollectionTree");

    std::cout << "File: " << filePath << "\n"
              << "Compression algorithm: "
              << treeFile->GetCompressionAlgorithm() << "\n"
              << "Compression level: " << treeFile->GetCompressionLevel()
              << "\n"
              << "Compression factor: " << treeFile->GetCompressionFactor()
              << "\n"
              << "File size " << collectionTree->GetZipBytes() << "\n\n";

    resultsFile << treeFile->GetCompressionSettings() << " "
                << collectionTree->GetZipBytes() << "\n";
  }

  resultsFile << std::endl;
  resultsFile.close();

  std::cout << std::endl;
  return 0;
}

int main() {
  // We ignore dictionary warnings (for now...)
  gErrorIgnoreLevel = kError;

  ttreeFileCompression();
  ttreeTypeCompression();

  return 0;
}
