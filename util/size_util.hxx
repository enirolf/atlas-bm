/**
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#ifndef UTIL_HXX
#define UTIL_HXX

#include <TGraph.h>

#include <Compression.h>

#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <string>

struct SizeStats {
  SizeStats(std::string k, std::uint64_t ne, std::uint64_t sd, std::uint64_t sm,
            int cs)
      : kind(k), nEntries(ne), sizeOnDisk(sd), sizeInMemory(sm),
        compressionSettings(cs) {}

  std::string kind;
  std::uint64_t nEntries;
  std::uint64_t sizeOnDisk;
  std::uint64_t sizeInMemory;
  int compressionSettings;

  void print() {
    std::cout << "Kind:\t\t\t\t" << this->kind << "\n"
              << "# Entries:\t\t\t" << this->nEntries << "\n"
              << "Total compressed bytes:\t\t" << this->sizeOnDisk << " B \n"
              << "Total uncompressed bytes:\t" << this->sizeInMemory << " B \n"
              << "Avg bytes / entry (compr.):\t"
              << float(this->sizeOnDisk) / float(this->nEntries) << " B \n"
              << "Avg bytes / entry (uncompr.):\t"
              << float(this->sizeInMemory) / float(this->nEntries) << " B \n"
              << "Compression settings:\t\t" << this->compressionSettings
              << "\n"
              << "Total compression factor:\t" << std::fixed
              << std::setprecision(2)
              << float(this->sizeInMemory) / float(this->sizeOnDisk) << "\n"
              << std::endl;
  }

  int writeToFile(std::fstream &resultsFile) {

    if (!resultsFile.is_open()) {
      std::cerr << "Failed to open results file" << std::endl;
      return 1;
    }

    resultsFile << this->kind << " " << this->nEntries << " "
                << this->sizeOnDisk << " " << this->sizeInMemory << " "
                << this->compressionSettings << std::endl;

    return 0;
  }
};

std::string getCompAlgName(Int_t alg);

void mkCompAlgoMap(std::map<Int_t, std::string> *algMap);

void removeSpaces(std::string *str);

#endif // UTIL_HXX
