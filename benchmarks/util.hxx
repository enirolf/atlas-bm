/**
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#ifndef UTIL_HXX
#define UTIL_HXX

#include <ROOT/RDataFrame.hxx>

#include <Compression.h>

#include <map>
#include <string>

std::string getCompAlgName(Int_t alg);

void setGraphStyle(TGraph *graph);

void mkCompAlgoMap(std::map<Int_t, std::string> *algMap);

void removeSpaces(std::string *str);

#endif // UTIL_HXX
