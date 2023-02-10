#include <iostream>
#include <string>

#include <TFile.h>
#include <TTree.h>

const std::string sourcePath =
    "data/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13167_p5169/DAOD_PHYS.29445526._000495.pool.root.1";
const std::string targetBasePath = "data/mc20_DAOD_PHYS.ttree.root";
const int compressionSettings[] = {0, 207, 404, 505};
const int nEntries = -1; // change to -1 to consider all entries

void write_trees() {
  std::cout << "Opening " << sourcePath << "..." << std::endl;
  auto sourceFile = TFile::Open(sourcePath.c_str());
  auto sourceTree = sourceFile->Get<TTree>("CollectionTree");

  for (auto setting : compressionSettings) {
    std::string targetPath = targetBasePath + "~" + std::to_string(setting);
    auto targetFile = TFile::Open(targetPath.c_str(), "RECREATE");
    targetFile->SetCompressionSettings(setting);

    std::cout << "Writing " << targetPath << "..." << std::endl;
    auto targetTree = sourceTree->CloneTree(nEntries);
    targetTree->Write();
  }
}
