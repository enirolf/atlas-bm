#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleInspector.hxx>

#include <iostream>
#include <regex>
#include <set>
#include <string>

using ROOT::Experimental::RNTuple;
using ROOT::Experimental::RNTupleInspector;

struct AuxDynFieldInfo {
   ROOT::Experimental::DescriptorId_t fieldId;
   std::string typeName;
};

int countIndexFields(std::string_view ntuplePath, std::string_view ntupleName) {
   auto inspector = RNTupleInspector::Create(ntupleName, ntuplePath);

   // FieldNameBase -> TypeName -> NumElems -> ID
   std::map<std::string, std::map<std::string, std::map<std::uint64_t, ROOT::Experimental::DescriptorId_t>>> auxDynVectorFields;
   std::regex fieldRegex(R"((.*)AuxDyn(.*))");

   for (const auto id : inspector->GetFieldsByName(fieldRegex)) {
      const auto &desc = inspector->GetFieldInfo(id).GetDescriptor();
      std::string fieldName = desc.GetFieldName();
      auto col = inspector->GetDescriptor()->FindPhysicalColumnId(id, 0);

      // std::string fieldNameBase = fieldName.substr(0, fieldName.find(':'));

      // std::cout << fieldNameBase << " "
      //           << desc.GetTypeName() << std::endl;

      // if (desc.GetTypeName() == "std::vector<float>") {
      //    auxDynVectorFields[fieldNameBase][desc.GetTypeName()][desc.GetNElems()] = desc.GetId();
      // }
   }

   return 0;
}


int main(int argc, char **argv)
{
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

  countIndexFields(ntuplePath, ntupleName);

  return 0;
}
