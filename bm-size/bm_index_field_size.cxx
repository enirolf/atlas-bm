#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleInspector.hxx>

#include <iostream>
#include <regex>
#include <set>
#include <string>

using ROOT::Experimental::RNTuple;
using ROOT::Experimental::RNTupleInspector;

const bool kUse32BitIndex = false;

struct IndexFieldInfo {
  std::uint64_t fieldId;
  std::string fieldNameSuffix;
  // std::uint64_t nIndexElems, indexSizeOnDisk, indexSizeInMem;
  std::uint64_t nDataElems, indexSizeOnDisk, indexSizeInMem;
};

typedef std::map<std::string, std::map<std::string, std::vector<IndexFieldInfo>>> IndexFieldStats_t;

IndexFieldStats_t countIndexFields(RNTupleInspector *inspector) {
  // FieldNameBase -> TypeName -> [IndexFieldInfo]
  std::map<std::string, std::map<std::string, std::vector<IndexFieldInfo>>> auxDynVectorFields;
  std::regex fieldRegex(R"((.*)Aux(Dyn:|::)(.*))");
  std::regex vecRegex(R"((std::vector<)+(float|std::u?int[0-9]+_t|char)>+)");

  for (const auto id : inspector->GetFieldsByName(fieldRegex)) {
    const auto &desc = inspector->GetFieldTreeInfo(id).GetDescriptor();
    std::string fieldName = desc.GetFieldName();

    if (std::regex_match(desc.GetTypeName(), vecRegex)) {
      auto indexColId = inspector->GetDescriptor()->FindPhysicalColumnId(id, 0);
      auto indexColInfo = inspector->GetColumnInfo(indexColId);

      auto dataField = inspector->GetFieldTreeInfo(fieldName + "._0");
      auto dataColId =
          inspector->GetDescriptor()->FindPhysicalColumnId(dataField.GetDescriptor().GetId(), 0);
      auto dataColInfo = inspector->GetColumnInfo(dataColId);

      auto info = IndexFieldInfo{desc.GetId(), fieldName.substr(fieldName.find(':')).substr(2),
                                 dataColInfo.GetNElements(), indexColInfo.GetOnDiskSize(),
                                 indexColInfo.GetInMemorySize()};

      auxDynVectorFields[fieldName.substr(0, fieldName.find(':'))][desc.GetTypeName()].emplace_back(
          info);
    }
  }

  return auxDynVectorFields;
}

void printIndexFieldStats(const IndexFieldStats_t &indexFieldStats) {
  for (const auto &[fieldBaseName, types] : indexFieldStats) {
    std::cout << fieldBaseName << std::endl;
    for (const auto &[typeName, fields] : types) {
      std::cout << "\t" << typeName << std::endl;
      for (const auto &info : fields) {
        std::cout << "\t\tcontainer = " << info.fieldNameSuffix
                  << ", in mem = " << info.indexSizeInMem << ", on disk = " << info.indexSizeOnDisk
                  << std::endl;
      }
    }
  }
}

void calcSpaceSavings(std::string_view ntuplePath, std::string_view ntupleName) {
  auto inspector = RNTupleInspector::Create(ntupleName, ntuplePath);

  const IndexFieldStats_t indexFieldStats = countIndexFields(inspector.get());

  std::uint64_t colsToSave = 0;
  std::uint64_t bytesToSave = 0;
  for (const auto &[fieldBaseName, types] : indexFieldStats) {
    for (const auto &[typeName, fields] : types) {
      auto nIndexFields = fields.size();
      auto indexSizeInMem = fields[0].indexSizeOnDisk;

      // std::cout << fieldBaseName << " (" << typeName << "):\n\t"
      //           << "# fields = " << nIndexFields
      //           << "\ttotal uncompr. size = " << indexSizeInMem * nIndexFields << " B" << std::endl;

      colsToSave += nIndexFields - 1;
      bytesToSave += indexSizeInMem * (nIndexFields - 1);
      // for (const auto &info : fields) {
      //   std::cout << "\t\tcontainer = " << info.fieldNameSuffix
      //             << ", in mem = " << info.indexSizeInMem << ", on disk = " <<
      //             info.indexSizeOnDisk
      //             << std::endl;
      // }
      printIndexFieldStats(indexFieldStats);
    }
  }

  std::vector<std::uint64_t> indexCols;

  if (kUse32BitIndex)
    indexCols = inspector->GetColumnsByType(ROOT::Experimental::EColumnType::kSplitIndex32);
  else
    indexCols = inspector->GetColumnsByType(ROOT::Experimental::EColumnType::kSplitIndex64);

  std::uint64_t totIndexColSize = 0;
  for (const auto id : indexCols) {
    totIndexColSize += inspector->GetColumnInfo(id).GetOnDiskSize();
  }

  std::cout << "total uncompressed bytes = " << inspector->GetOnDiskSize() << std::endl;
  std::cout << "total # index columns    = " << indexCols.size() << "\ttotal index col bytes\t= " << totIndexColSize << std::endl;
  std::cout << "redundant index columns  = " << colsToSave << "\tbytes to save\t= " << bytesToSave << std::endl;
  std::cout << "potential percentage of uncompressed bytes to save (wrt. total size) = " << (float)bytesToSave / (float)inspector->GetInMemorySize() * 100 << std::endl;
}

int main(int argc, char **argv) {
  // Suppress (irrelevant) warnings
  gErrorIgnoreLevel = kError;

  std::string ntuplePath;
  std::string ntupleName = "RNT:CollectionTree";

  int c;
  while ((c = getopt(argc, argv, "hi:n:s:")) != -1) {
    switch (c) {
    case 'i':
      ntuplePath = optarg;
      break;
    case 'n':
      ntupleName = optarg;
      break;
    default:
      return 1;
    }
  }

  if (ntuplePath == "") {
    std::cerr << "ERROR: please provide an input path\n" << std::endl;
    return 1;
  }

  calcSpaceSavings(ntuplePath, ntupleName);
  // printIndexFieldStats(indexFieldStats);

  return 0;
}
