#include <TColor.h>
#include <TGraphErrors.h>

#include <cstdlib>

enum EnumGraphTypes {
  kGraphTreeDirect,
  kGraphTreeOptDirect,
  kGraphNtupleDirect,
  kGraphRatioDirect,
  kNumGraphs
};

enum EnumCompression { kZipNone, kZipLz4, kZipZstd, kZipZlib, kZipLzma };
const char *kCompressionNames[] = {"uncompressed", "lz4", "zstd", "zlib", "lzma"};

enum EnumPlotColors { kPlotRed = kRed - 3, kPlotBlue = kBlue - 3, kPlotGreen = kGreen - 2 };

const std::map<std::string, int> colors{{"ttree", TColor::GetColor(2, 103, 193)},
                                        {"rntuple", TColor::GetColor(200, 62, 77)},
                                        {"rntuple_uring", TColor::GetColor(59, 178, 115)},
                                        {"fill", TColor::GetColor(0, 0, 0)}};
const std::map<std::string, int> styles{
    {"ttree", 1001},         {"rntuple", 1001},          {"rntuple_mt", 3001},
    {"rntuple_uring", 1001}, {"rntuple_mt_uring", 3008}, {"fill", 1001}};

float StdErr(ROOT::VecOps::RVec<float> vals) {
  int nVal = vals.size();
  float mean = ROOT::VecOps::Mean(vals);
  float s2 = ROOT::VecOps::Var(vals);
  float s = sqrt(s2);
  float t = abs(ROOT::Math::tdistribution_quantile(0.05 / 2., nVal - 1));
  return t * s / sqrt(nVal);
}

void SetStyle() {
  gStyle->SetEndErrorSize(6);
  gStyle->SetOptTitle(1);
  gStyle->SetOptStat(0);
}

Int_t GetTransparentColor() { return 1179; }

int getXVal(std::string_view format, int compression) {
  int x;
  if (format == "filler")
    x = 0;
  if (format == "ttree")
    x = 1;
  else if (format == "rntuple")
    x = 2;
  else if (format == "rntuple_uring")
    x = 3;

  switch (compression) {
  case 0:
    x += 0;
    break;
  case 505:
    x += 4;
    break;
  case 201:
    x += 8;
    break;
  case 207:
    x += 12;
    break;
  }
  return x;
}
