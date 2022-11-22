/**
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#ifndef UTIL_HXX
#define UTIL_HXX

#include <ROOT/RDataFrame.hxx>

#include <Compression.h>

#include <map>
#include <string>

namespace constants {
  inline extern const std::map<Int_t, std::string> compressionAlgorithms = {
      {ROOT::RCompressionSetting::EAlgorithm::kZLIB, "ZLIB"},
      {ROOT::RCompressionSetting::EAlgorithm::kLZMA, "LZMA"},
      {ROOT::RCompressionSetting::EAlgorithm::kLZ4, "LZ4"},
      {ROOT::RCompressionSetting::EAlgorithm::kZSTD, "ZSTD"}};

  inline extern const Int_t canvasWidth = -1200;
  inline extern const Int_t canvasHeight = 700;
} // constants

void setPlotStyle(TGraph *plot);

#endif // UTIL_HXX
