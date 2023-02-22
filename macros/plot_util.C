enum EnumGraphTypes { kGraphTreeDirect, kGraphTreeOptDirect, kGraphNtupleDirect, kGraphRatioDirect, kNumGraphs };

enum EnumCompression { kZipNone, kZipLz4, kZipZstd, kZipZlib, kZipLzma };
const char *kCompressionNames[] = {"uncompressed", "lz4", "zstd", "zlib", "lzma"};

struct TypeProperties {
  TypeProperties() : graph(NULL), color(0), shade(0) { };
  TypeProperties(TGraphErrors *g, int c, int s, bool r, bool d)
    : graph(g), color(c), shade(s), is_ratio(r), is_direct(d) { }

  TGraphErrors *graph;
  int color;
  int shade;
  bool is_ratio = false;
  bool is_direct = false;
};

struct GraphProperties {
  GraphProperties()
    : type(kGraphTreeDirect), compression(kZipNone), priority(-1), size(0.0) { }
  GraphProperties(EnumGraphTypes ty, EnumCompression c)
    : type(ty)
    , compression(c)
    , priority(kNumGraphs * compression + type)
    , size(0.0) { }

  EnumGraphTypes type;
  EnumCompression compression;
  int priority;
  float size;
};

void FillPropsMap(std::map<std::string, GraphProperties> *props_map) {
  (*props_map)["TTree_none"] =
   GraphProperties(kGraphTreeDirect, kZipNone);
  (*props_map)["TTree_zstd"] =
   GraphProperties(kGraphTreeDirect, kZipZstd);
  (*props_map)["TTreeOpt_none"] =
   GraphProperties(kGraphTreeOptDirect, kZipNone);
  (*props_map)["TTreeOpt_zstd"] =
   GraphProperties(kGraphTreeOptDirect, kZipZstd);
  (*props_map)["RNTuple_none"] =
   GraphProperties(kGraphNtupleDirect, kZipNone);
  (*props_map)["RNTuple_zstd"] =
   GraphProperties(kGraphNtupleDirect, kZipZstd);
}

void FillGraphMap(std::map<EnumGraphTypes, TypeProperties> *graph_map) {
  (*graph_map)[kGraphTreeDirect] =
    TypeProperties(new TGraphErrors(), kBlue, 1001, false, true);
  (*graph_map)[kGraphTreeOptDirect] =
    TypeProperties(new TGraphErrors(), kGreen + 2, 1001, false, true);
  (*graph_map)[kGraphNtupleDirect] =
    TypeProperties(new TGraphErrors(), kRed, 1001, false, true);
  (*graph_map)[kGraphRatioDirect] =
    TypeProperties(new TGraphErrors(), kGreen + 2, 1001, true, false);
  // (*graph_map)[kGraphTreeRdf] =
  //   TypeProperties(new TGraphErrors(), kBlue, 3001, false, false);
  // (*graph_map)[kGraphNtupleRdf] =
  //   TypeProperties(new TGraphErrors(), kRed, 3001, false, false);
  // (*graph_map)[kGraphRatioDirect] =
  //   TypeProperties(new TGraphErrors(), kGreen + 2, 1001, true, false);
  // (*graph_map)[kGraphRatioRdf] =
  //   TypeProperties(new TGraphErrors(), kGreen + 2, 3001, true, false);
}

TString GetPhysicalFormat(TString format) {
  TObjArray *tokens = format.Tokenize("~");
  TString physical_format =
    reinterpret_cast<TObjString *>(tokens->At(0))->CopyString();
  delete tokens;
  return physical_format;
}

float GetBloatFactor(TString format) {
  if (format.EndsWith("times10"))
    return 10.0;
  return 1.0;
}

// void GetStats(float *vals, int nval, float &mean, float &error) {
//   assert(nval > 1);
//   mean = 0.0;
//   for (int i = 0; i < nval; ++i)
//     mean += vals[i];
//   mean /= nval;
//   float s2 = 0.0;
//   for (int i = 0; i < nval; ++i)
//     s2 += (vals[i] - mean) * (vals[i] - mean);
//   s2 /= nval - 1;
//   float s = sqrt(s2);
//   float t = abs(ROOT::Math::tdistribution_quantile(0.05 / 2., nval - 1));
//   error = t * s / sqrt(nval);
// }

void SetStyle() {
  gStyle->SetEndErrorSize(6);
  gStyle->SetOptTitle(1);
  gStyle->SetOptStat(0);
  //gStyle->SetTitleFontSize(30);

  Int_t ci = 1179;      // for color index setting
  new TColor(ci, 1, 0, 0, " ", 0.);
}

Int_t GetTransparentColor() {
  return 1179;
}
