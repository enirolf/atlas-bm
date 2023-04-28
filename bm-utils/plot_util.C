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
                                        {"ratio", TColor::GetColor(59, 178, 115)},
                                        {"filler", TColor::GetColor(0, 0, 0)}};
const std::map<std::string, int> styles{
    {"ttree", 1001},         {"rntuple", 1001},          {"rntuple_mt", 3001},
    {"rntuple_uring", 1001}, {"rntuple_mt_uring", 3008}, {"filler", 1001}};

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

int getXVal(std::string_view format, int compression, bool withUring) {
  int factor = withUring ? 4 : 3;

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
    x += 0 * factor;
    break;
  case 505:
    x += 1 * factor;
    break;
  case 201:
    x += 2 * factor;
    break;
  case 207:
    x += 3 * factor;
    break;
  }
  return x;
}
