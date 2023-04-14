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

const std::map<std::string, int> colors{{"ttree", kPlotBlue},           {"rntuple", kPlotRed},
                                        {"rntuple_mt", kPlotRed},       {"rntuple_uring", kPlotRed},
                                        {"rntuple_mt_uring", kPlotRed}, {"fill", kWhite}};
const std::map<std::string, int> styles{
    {"ttree", 1001},         {"rntuple", 1001},          {"rntuple_mt", 3001},
    {"rntuple_uring", 3144}, {"rntuple_mt_uring", 3008}, {"fill", 1001}};

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
  // gStyle->SetTitleFontSize(30);

  Int_t ci = 1179; // for color index setting
  new TColor(ci, 1, 0, 0, " ", 0.);
}

Int_t GetTransparentColor() { return 1179; }
