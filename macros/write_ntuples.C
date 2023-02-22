#include <iostream>
#include <string>

#include <ROOT/RNTupleImporter.hxx>

using ROOT::Experimental::RNTupleImporter;

const std::string sourceBasePath = "data/DAOD_PHYS_DATA.ttree.root";
const std::string targetBasePath = "data/DAOD_PHYS_DATA.rntuple.root";
const int compressionSettings[] = {0, 505};

void write_ntuples() {
  gErrorIgnoreLevel = kError;
  gInterpreter->ProcessLine(
      "#include \"xAODMissingET/versions/MissingETCompositionBase.h\"");

  for (auto setting : compressionSettings) {
    std::string sourcePath = sourceBasePath + "~" + std::to_string(setting);
    std::string targetPath = targetBasePath + "~" + std::to_string(setting);

    std::filesystem::remove(targetPath);

    auto importer =
        RNTupleImporter::Create(sourcePath, "CollectionTree", targetPath)
            .Unwrap();
    importer->SetNTupleName("CollectionNTuple");
    importer->SetIsQuiet(true);
    auto writeOptions = importer->GetWriteOptions();
    writeOptions.SetCompression(setting);
    importer->SetWriteOptions(writeOptions);

    std::cout << "Importing to " << targetPath << "..." << std::endl;
    importer->Import();
  }
}
