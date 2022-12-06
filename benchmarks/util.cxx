/**
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <algorithm>

#include "util.hxx"

std::string getCompAlgName(Int_t alg) {
  switch (alg) {
  case ROOT::RCompressionSetting::EAlgorithm::kZLIB:
    return "ZLIB";
    break;
  case ROOT::RCompressionSetting::EAlgorithm::kLZMA:
    return "LZMA";
    break;
  case ROOT::RCompressionSetting::EAlgorithm::kLZ4:
    return "LZ4";
    break;
  case ROOT::RCompressionSetting::EAlgorithm::kZSTD:
    return "ZSTD";
    break;
  default:
    return "UNKNOWN";
  }
}

void mkCompAlgoMap(std::map<Int_t, std::string> *algMap) {
  (*algMap)[0] = "uncompressed";
  (*algMap)[105] = "zlib";
  (*algMap)[207] = "lzma";
  (*algMap)[404] = "lz4";
  (*algMap)[505] = "zstd";
}

void removeSpaces(std::string *str) {
  str->erase(remove_if(str->begin(), str->end(), isspace), str->end());
}
