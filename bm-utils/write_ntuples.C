#include <iostream>
#include <string>

#include <ROOT/RNTupleImporter.hxx>

using ROOT::Experimental::RNTupleImporter;

void import(std::string sourcePath, std::string targetPath, int compression = 505) {
  std::filesystem::remove(targetPath);

  auto importer = RNTupleImporter::Create(sourcePath, "CollectionTree", targetPath);
  importer->SetIsQuiet(true);
  importer->SetConvertDotsInBranchNames(true);
  auto writeOptions = importer->GetWriteOptions();
  writeOptions.SetCompression(setting);
  importer->SetWriteOptions(writeOptions);

  std::cout << "Importing to " << targetPath << "..." << std::endl;
  importer->Import();
}

void write_ntuples() {
  const int compressionSettings[] = {201, 207, 404, 505};

  const std::string sourcePathBaseMC = "data/mc/DAOD_PHYS.ttree.root";
  const std::string targetPathBaseMC = "data/mc/DAOD_PHYS.rntuple.root";

  const std::string sourcePathBaseData = "data/data/DAOD_PHYS.ttree.root";
  const std::string targetPathBaseData = "data/data/DAOD_PHYS.rntuple.root";

  for (auto c : compressionSettings) {
    import(sourceBasePathMC + "~" + std::to_string(c), targetBasePathMC + "~" + std::to_string(c));
    import(sourceBasePathData + "~" + std::to_string(c), targetBasePathData + "~" + std::to_string(c));
  }
}
