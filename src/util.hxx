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
// inline extern const std::map<Int_t, std::string> compressionAlgorithms = {
//       {0, "uncompressed"},
//       {105, "zlib"},
//       {207, "lzma"},
//       {404, "lz4"},
//       {505, "zstd"}
//     };

inline extern const Int_t canvasWidth = -1200;
inline extern const Int_t canvasHeight = 700;
} // namespace constants

std::string getCompAlgName(Int_t alg);

void setGraphStyle(TGraph *graph);

void mkCompAlgoMap(std::map<Int_t, std::string> *algMap);

void removeSpaces(std::string *str);

#endif // UTIL_HXX
