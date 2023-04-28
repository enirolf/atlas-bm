#include <iostream>
#include <string>

#include <ROOT/RNTupleImporter.hxx>

using ROOT::Experimental::RNTupleImporter;

// const int compressionSettings[] = {0, 505, 201, 207};
const int compressionSettings[] = {505};

void import(std::string sourceBasePath, std::string targetBasePath, std::size_t clusterSize) {
  gErrorIgnoreLevel = kError;
  gInterpreter->ProcessLine("#include <xAODMissingET/versions/MissingETCompositionBase.h>");
  gInterpreter->ProcessLine("#include <xAODMissingET/versions/MissingETBase.h>");
  gInterpreter->ProcessLine("#include <CxxUtils/sgkey_t.h>");

  for (auto setting : compressionSettings) {
    std::string sourcePath = sourceBasePath + "~" + std::to_string(setting);
    std::string targetPath = targetBasePath + "~" + std::to_string(setting);

    std::filesystem::remove(targetPath);

    auto importer = RNTupleImporter::Create(sourcePath, "CollectionTree", targetPath).Unwrap();
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
  import("data/daod_phys_benchmark_files/data/DAOD_PHYS.ttree.root",
         "data/misc/DAOD_PHYS.rntuple100.root", 100 * 1000 * 1000);
  import("data/daod_phys_benchmark_files/data/DAOD_PHYS.ttree.root",
         "data/misc/DAOD_PHYS.rntuple150.root", 150 * 1000 * 1000);
  // import("data/daod_phys_benchmark_files/mc/DAOD_PHYS.ttree.root",
  //        "data/daod_phys_benchmark_files/mc/DAOD_PHYS.rntuple.root");
}
