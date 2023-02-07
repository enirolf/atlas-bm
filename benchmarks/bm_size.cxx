#include <TFile.h>
#include <TTree.h>

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleInspector.hxx>

#include <iostream>
#include <vector>

#include "util/size_util.hxx"

using ROOT::Experimental::RNTuple;
using ROOT::Experimental::RNTupleInspector;

const int kCompressionSettings[] = {0, 207, 404, 505};

std::unique_ptr<SizeStats>
bmNTupleSize(const std::string ntuplePath,
             const std::string ntupleName = "CollectionNTuple") {
  auto file = std::unique_ptr<TFile>(TFile::Open(ntuplePath.c_str()));
  auto ntuple = file->Get<RNTuple>(ntupleName.c_str());
  auto inspector = RNTupleInspector::Create(ntuple).Unwrap();

  return std::make_unique<SizeStats>(
      "RNTuple", inspector->GetNEntries(), inspector->GetCompressedSize(),
      inspector->GetUncompressedSize(), inspector->GetCompressionSettings());
}

std::unique_ptr<SizeStats>
bmTreeSize(const std::string treePath,
           const std::string treeName = "CollectionTree") {
  auto file = std::unique_ptr<TFile>(TFile::Open(treePath.c_str()));
  auto tree = file->Get<TTree>(treeName.c_str());

  return std::make_unique<SizeStats>("TTree", tree->GetEntries(),
                                     tree->GetZipBytes(), tree->GetTotBytes(),
                                     file->GetCompressionSettings());
}

int main() {
  // Suppress warnings
  gErrorIgnoreLevel = kError;

  std::fstream resultsFile;
  resultsFile.open("results/size_mc16.txt", std::ios_base::out);

  for (const auto setting : kCompressionSettings) {
    std::cout << "### Compression = " << setting << " ###" << std::endl;
    const std::string treePath =
        "data/mc16_DAOD_PHYS.ttree.root~" + std::to_string(setting);
    auto treeStats = bmTreeSize(treePath);
    treeStats->print();
    treeStats->writeToFile(resultsFile);

    // const std::string ntuplePath =
    //     "data/PHYS_FULL.rntuple.root~" + std::to_string(setting);
    // auto ntupleStats = bmNTupleSize(ntuplePath);
    // ntupleStats->print();
    // ntupleStats->writeToFile(resultsFile);
  }

  resultsFile.close();

  return 0;
}
