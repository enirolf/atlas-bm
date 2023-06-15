#include <iostream>
#include <string>

#include <ROOT/RNTupleImporter.hxx>

using ROOT::Experimental::RNTupleImporter;

const int compressionSettings[] = {0,201,207,404,505};

void import(std::string sourceBasePath, std::string targetBasePath, std::size_t clusterSize=50*1000*1000) {
  gErrorIgnoreLevel = kError;
  gInterpreter->ProcessLine("#include <xAODMissingET/versions/MissingETCompositionBase.h>");
  gInterpreter->ProcessLine("#include <xAODMissingET/versions/MissingETBase.h>");
  gInterpreter->ProcessLine("#include <CxxUtils/sgkey_t.h>");

  for (auto setting : compressionSettings) {
    std::string sourcePath = sourceBasePath + "~" + std::to_string(setting);
    std::string targetPath = targetBasePath + "~" + std::to_string(setting);

    std::filesystem::remove(targetPath);

    auto importer = RNTupleImporter::Create(sourcePath, "CollectionTree", targetPath);
    importer->SetNTupleName("RNT:CollectionTree");
    importer->SetIsQuiet(true);
    auto writeOptions = importer->GetWriteOptions();
    writeOptions.SetCompression(setting);
    writeOptions.SetApproxZippedClusterSize(clusterSize);
    importer->SetWriteOptions(writeOptions);

    std::cout << "Importing to " << targetPath << "..." << std::endl;
    importer->Import();
  }
}

void write_ntuples() {
  import("data/daod_phys_benchmark_files/data/DAOD_PHYS.ttree.root", "data/daod_phys_benchmark_files/data/DAOD_PHYS.rntuple.root");
  import("data/daod_phys_benchmark_files/mc/DAOD_PHYS.ttree.root", "data/daod_phys_benchmark_files/mc/DAOD_PHYS.rntuple.root");
}
