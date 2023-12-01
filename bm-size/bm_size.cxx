#include <TFile.h>
#include <TTree.h>

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleInspector.hxx>

#include <iostream>
#include <vector>

using ROOT::Experimental::RNTuple;
using ROOT::Experimental::RNTupleInspector;

void bmNTupleSize(const std::string ntuplePath, const std::string ntupleName) {
  auto file = std::unique_ptr<TFile>(TFile::Open(ntuplePath.c_str()));
  auto ntuple = file->Get<RNTuple>(ntupleName.c_str());
  auto inspector = RNTupleInspector::Create(ntuple);
  auto descriptor = inspector->GetDescriptor();

  std::cout << "rntuple\t" << inspector->GetCompressionSettings() << "\t"
            << descriptor->GetNEntries() << "\t" << descriptor->GetNFields() << "\t"
            << inspector->GetCompressedSize() << "\t" << inspector->GetUncompressedSize() << std::endl;
}

void bmTreeSize(const std::string treePath, const std::string treeName) {
  std::unique_ptr<TFile> file;

  file = std::unique_ptr<TFile>(TFile::Open(treePath.c_str()));

  auto tree = file->Get<TTree>(treeName.c_str());

  std::cout << "ttree\t" << file->GetCompressionSettings() << "\t" << tree->GetEntries() << "\t"
            << tree->GetNbranches() << "\t" << tree->GetZipBytes() << "\t" << tree->GetTotBytes()
            << std::endl;
}

static void printUsage(std::string_view prog) {
  std::cout << "USAGE: " << prog << " (-h|-i INPUT_PATH -s (ttree|rntuple) [-n STORE_NAME])" << std::endl;
}

int main(int argc, char **argv) {
  // Suppress (irrelevant) warnings
  gErrorIgnoreLevel = kError;

  std::string inputPath;
  std::string storeName = "CollectionTree";
  bool isRNTuple = false;

  int c;
  while ((c = getopt(argc, argv, "hi:n:s:")) != -1) {
    switch (c) {
    case 'h':
      printUsage(argv[0]);
      return 0;
    case 'i':
      inputPath = optarg;
      break;
    case 'n':
      storeName = optarg;
      break;
    case 's':
      if (strcmp(optarg, "rntuple") == 0) {
        isRNTuple = true;
      } else if (strcmp(optarg, "ttree") != 0) {
        std::cerr << "ERROR: Unknown storage mode " << optarg << std::endl;
        return 1;
      }
      break;
    default:
      printUsage(argv[0]);
      return 1;
    }
  }

  if (inputPath == "") {
    std::cerr << "ERROR: please provide an input path\n" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  if (storeName == "") {
    std::cerr << "ERROR: please provide the name of the TTree/RNTuple to read\n" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  if (isRNTuple) {
    bmNTupleSize(inputPath, storeName);
  } else {
    bmTreeSize(inputPath, storeName);
  }

  return 0;
}
