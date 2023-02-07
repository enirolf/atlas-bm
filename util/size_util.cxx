/**
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

// #include <algorithm>
// #include <map>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>

#include "size_util.hxx"

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

void printSizeStats(SizeStats *stats) {
  std::cout << "Kind:\t\t\t\t" << stats->kind << "\n"
            << "# Entries:\t\t\t" << stats->nEntries << "\n"
            << "Total compressed bytes:\t\t" << stats->sizeOnDisk << " B \n"
            << "Total uncompressed bytes:\t" << stats->sizeInMemory << " B \n"
            << "Avg bytes / entry (compr.):\t"
            << float(stats->sizeOnDisk) / float(stats->nEntries) << " B \n"
            << "Avg bytes / entry (uncompr.):\t"
            << float(stats->sizeInMemory) / float(stats->nEntries) << " B \n"
            << "Compression settings:\t\t" << stats->compressionSettings << "\n"
            << "Total compression factor:\t" << std::fixed
            << std::setprecision(2)
            << float(stats->sizeInMemory) / float(stats->sizeOnDisk) << "\n"
            << std::endl;
}

void writeSizeStats(SizeStats *stats, std::fstream *resultsFile) {
  (*resultsFile) << stats->kind << " " << stats->nEntries << " "
                 << stats->sizeOnDisk << " " << stats->sizeInMemory << " "
                 << stats->compressionSettings << std::endl;
}
