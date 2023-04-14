#include <TFile.h>
#include <TTree.h>

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleInspector.hxx>

#include <iostream>
#include <vector>

using ROOT::Experimental::RNTuple;
using ROOT::Experimental::RNTupleInspector;
using ROOT::Experimental::RNTupleReader;

void bmNTupleFloatVecSize(const std::string ntuplePath, const std::string ntupleName) {
  auto file = std::unique_ptr<TFile>(TFile::Open(ntuplePath.c_str()));
  auto ntuple = file->Get<RNTuple>(ntupleName.c_str());
  auto inspector = RNTupleInspector::Create(ntuple);
  auto descriptor = inspector->GetDescriptor();

  std::uint64_t sizeOnDisk = 0;
  std::uint64_t sizeInMemory = 0;

  for (const auto &fld : descriptor->GetTopLevelFields()) {
    if (fld.GetTypeName().compare("std::vector<float>") == 0) {
      auto fieldInfo = inspector->GetFieldInfo(fld.GetId());
      sizeOnDisk += fieldInfo.GetOnDiskSize();
      sizeInMemory += fieldInfo.GetInMemorySize();
    }
  }

  std::cout << "rntuple " << descriptor->GetNEntries() << " "
            << inspector->GetFieldTypeCount("std::vector<float>") << " " << sizeOnDisk << " "
            << sizeInMemory << std::endl;
}

void bmTreeFloatVecSize(const std::string treePath, const std::string treeName) {
  auto file = std::unique_ptr<TFile>(TFile::Open(treePath.c_str()));
  auto tree = file->Get<TTree>(treeName.c_str());

  std::uint64_t nColumns = 0;
  std::uint64_t sizeOnDisk = 0;
  std::uint64_t sizeInMemory = 0;

  for (auto br : TRangeDynCast<TBranch>(tree->GetListOfBranches())) {
    if (!br)
      continue;

    if (strcmp(br->GetClassName(), "vector<float>") == 0) {
      nColumns++;
      sizeOnDisk += br->GetZipBytes();
      sizeInMemory += br->GetTotBytes();
    }
  }

  std::cout << "ttree " << tree->GetEntries() << " " << nColumns << " " << sizeOnDisk << " "
            << sizeInMemory << std::endl;
}

static void printUsage(std::string_view prog) {
  std::cout << "USAGE: " << prog << " -i INPUT_PATH -n STORE_NAME -m (ttree|rntuple)" << std::endl;
}

int main(int argc, char **argv) {
  // Suppress (irrelevant) warnings
  gErrorIgnoreLevel = kError;

  std::string inputPath, storeName;
  bool checkNTuple = false;

  int c;
  while ((c = getopt(argc, argv, "hi:n:m:")) != -1) {
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
    case 'm':
      if (strcmp(optarg, "rntuple") == 0) {
        checkNTuple = true;
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

  if (checkNTuple) {
    bmNTupleFloatVecSize(inputPath, storeName);
  } else {
    bmTreeFloatVecSize(inputPath, storeName);
  }

  return 0;
}
