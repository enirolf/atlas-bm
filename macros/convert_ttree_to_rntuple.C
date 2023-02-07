#include <iostream>
#include <string>

#include <ROOT/RNTupleImporter.hxx>

using RNTupleImporter = ROOT::Experimental::RNTupleImporter;

std::string kTreeFileName = "data/ATLAS_DAOD.ttree.root";
std::string kTreeName = "CollectionTree";
std::string kNTupleFileName = "data/ATLAS_DAOD.rntuple.root";
std::string kNTupleName = "CollectionNTuple";

void replace_all(std::string &inout, std::string_view what,
                 std::string_view with) {
  for (std::string::size_type pos{};
       inout.npos != (pos = inout.find(what.data(), pos, what.length()));
       pos += with.length()) {
    inout.replace(pos, what.length(), with.data(), with.length());
  }
}

void make_valid_branchnames(TTree *tree) {
  for (auto b : TRangeDynCast<TBranch>(tree->GetListOfBranches())) {
    if (!b) {
      continue;
    }

    std::string branchName = b->GetName();
    replace_all(branchName, ".", "-");
    b->SetName(branchName.c_str());
  }

  tree->Write();
}

void convert_ttree_to_rntuple() {

  auto treeFile = TFile::Open(kTreeFileName.c_str());

  int compressionSettings = treeFile->GetCompressionSettings();

  gInterpreter->ProcessLine(
      "#include \"xAODMissingET/versions/MissingETCompositionBase.h\"");

  std::cout << "Creating importer for file: " << kTreeFileName << std::endl;
  auto importer =
      RNTupleImporter::Create(kTreeFileName, kTreeName, kNTupleFileName)
          .Unwrap();
  importer->SetNTupleName(kNTupleName);
  importer->SetIsQuiet(false);
  auto writeOptions = importer->GetWriteOptions();
  writeOptions.SetCompression(compressionSettings);
  std::cout << "Importing..." << std::endl;
  importer->Import();
  std::cout << "Done!" << std::endl;
}
