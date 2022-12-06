#define COLOR_TRANSPARANT 1179

template <typename T, std::size_t N>
int FindIndex(const std::array<T, N> &arr, T elem) {
  if (int i =
          std::distance(begin(arr), std::find(begin(arr), end(arr), elem))) {
    return i;
  } else {
    return -1;
  }
}

typedef int CompressionSetting;

inline const std::array compressionSettings{0, 105, 207, 404, 505};
inline const std::array compressionLabels{"uncompressed", "zlib", "lzma", "lz4",
                                          "zstd"};
inline const std::array compressionColors{kRed, kBlue, kOrange, kPink, kGreen};

typedef std::string ContainerName;
typedef std::string CategoryName;

struct ContainerProperties {
  ContainerProperties(std::string b, std::string c, float d)
      : branchName(b), branchClass(c), diskSize(d) {}

  std::string branchName;
  std::string branchClass;
  float diskSize;
};

struct CategoryProperties {
  CategoryProperties(CompressionSetting s, int i)
      : compressionSetting(s), stepSize(i), containerProperties({}) {}

  CompressionSetting compressionSetting;
  int stepSize;
  std::map<ContainerName, std::vector<ContainerProperties>> containerProperties;
};

typedef std::map<CategoryName, CategoryProperties *> CategorySizeMap;
typedef std::map<CategoryName, TGraph *> CategoryGraphMap;

inline const std::array categories{"MetaData",
                                   "Trig",
                                   "MET",
                                   "EvtId",
                                   "tau",
                                   "PFO",
                                   "egamma",
                                   "muon",
                                   "BTag",
                                   "HGTD",
                                   "InDet",
                                   "ITK",
                                   "Jet",
                                   "CaloTopo",
                                   "Calo",
                                   "Truth",
                                   "AFP",
                                   "LRT",
                                   "caloringer",
                                   "AnalysisElectrons",
                                   "AnalysisTauJets",
                                   "AnalysisPhotons",
                                   "AnalysisMuons",
                                   "AnalysisJets",
                                   "AnalysisHLT",
                                   "AnalysisTrigMatch",
                                   "AnalysisLargeRJets"};

void InitCategorySizeMap(CategorySizeMap *catSizeMap,
                         CompressionSetting setting, int settingIndex) {
  for (const auto &catName : categories) {
    (*catSizeMap)[catName] = new CategoryProperties(setting, settingIndex);
  }
}

void InitCategoryGraphMap(CategoryGraphMap *catGraphMap) {
  for (const auto &catName : categories) {
    (*catGraphMap)[catName] = new TGraph();
  }
}

inline const std::map<CategoryName, std::regex> categoryPatterns{
    {"MetaData", std::regex("^DataHeader|(.*)_mems$|(.*)_timings$|^Token$|^"
                            "RawInfoSummaryForTag$|^index_ref$")},
    {"Trig", std::regex("^HLT|^LVL1|^L1|^xTrig|^Trig|^CTP_Decision|^"
                        "TrigInDetTrackTruthMap|^"
                        "TrigNavigation|.*TriggerTowers|TileTTL1MBTS|^"
                        "TileL2Cnt|RoIBResult|^_"
                        "TRIGGER|^L1TopoRawData|BunchConfKey")},
    {"MET", std::regex("^MET|^METMAP|JEMEtSums")},
    {"EvtId",
     std::regex("^ByteStreamEventInfo|^EventInfo|^McEventInfo|^LumiBlockN|^"
                "EventWeight|^RunNumber|^ConditionsRun|^EventTime|^BunchId|^"
                "EventNumber",
                "^IsTestBeam|^IsSimulation|^IsCalibration|^AvgIntPerXing|^"
                "ActualIntPerXing|^RandomNumber|^McChannel")},
    {"tau", std::regex("^Tau|^DiTauJets")},
    {"PFO",
     std::regex("(.*)EventShape$|^AntiKt4EMPFlowJets|^"
                "JetETMissChargedParticleFlowObjects|^"
                "JetETMissNeutralParticleFlowObjects|^CHS(.*)"
                "ChargedParticleFlowObjects|^CHSNeutralParticleFlowObjects|^"
                "JetETMissLCNeutralParticleFlowObjects|^Global(.*)"
                "ParticleFlowObjects")},
    {"egamma", std::regex("^GSF|^ForwardElectron|^egamma|^Electron(?!.*Ring)|^"
                          "Photon(?!.*Ring)")},
    {"muon", std::regex("^Muon|^TileMuObj|^MS|^SlowMuons|.*Stau|(.*)"
                        "MuonTrackParticles$"
                        "|MUCTPI_RDO|^RPC|^TGC|^MDT|^CSC|^sTGC|^MM|.*"
                        "MuonMeasurements$|"
                        "^ExtrapolatedMuonTracks|^CombinedMuonTracks|^NCB_"
                        "MuonSegments|"
                        "^UnAssocMuonSegments|^EMEO_Muons|^EMEO_"
                        "MuonSpectrometerTrackParticles|^xAODNSWSegments")},
    {"BTag", std::regex("^BTag")},
    {"HGTD", std::regex("^HGTD")},
    {"InDet", std::regex("^InDet|^PrimaryVertices|^ComTime_TRT|^Pixel|^"
                         "TRT|^SCT|^BCM|^CTP|^"
                         "Tracks|^ResolvedForwardTracks|^"
                         "SplitClusterAmbiguityMap|^SoftBVrt")},
    {"ITk", std::regex("^ITk")},
    {"Jet", std::regex("^CamKt|^AntiKt|^Jet(?!.*ParticleFlowObjects$)|^"
                       "LCOriginTopoClusters|^EMOriginTopoClusters")},
    {"CaloTopo", std::regex("CaloCalTopoCluster|CaloCalFwdTopoTowers")},
    {"Calo", std::regex("^LAr|^AllCalo|^AODCellContainer|^MBTSContainer|^"
                        "CaloCompactCellContainer|^CaloEntryLayer|^"
                        "E4prContainer|^TileHitVec|^TileCellVec|^TileDigits")},
    {"Truth", std::regex("^Truth|Truth$|TruthMap$|TruthCollection$|^"
                         "PRD_MultiTruth|TracksTruth$"
                         "|.*TrackTruth$|TrackTruthCollection|^"
                         "HardScatter|BornLeptons")},
    {"AFP", std::regex("^AFP")},
    {"LRT", std::regex("^LRT|(.*)LRT$|(.*)LRTTrackParticles$|(.*)"
                       "LargeD0TrackParticles$")},
    {"caloringer", std::regex("(.*)Ring")},
    {"AnalysisElectrons", std::regex("^AnalysisElectrons")},
    {"AnalysisTauJets", std::regex("^AnalysisTauJets")},
    {"AnalysisPhotons", std::regex("^AnalysisPhotons")},
    {"AnalysisMuons", std::regex("^AnalysisMuons")},
    {"AnalysisJets", std::regex("^AnalysisJets")},
    {"AnalysisHLT", std::regex("^AnalysisHLT")},
    {"AnalysisTrigMatch", std::regex("^AnalysisTrigMatch")},
    {"AnalysisLargeRJets", std::regex("^AnalysisLargeRJets")},
};

CategoryName GetCategory(ContainerName containerName) {
  for (const auto &[catName, containerRegex] : categoryPatterns) {
    if (std::regex_search(containerName, containerRegex)) {
      return catName;
    }
  }

  return "UNKNOWN";
}

ContainerName GetContainerName(std::string branchName) {
  std::regex baseRegex("(.*)Aux(Dyn)?\..*");
  std::smatch res;

  if (std::regex_match(branchName, res, baseRegex)) {
    return res[1];
  }

  return branchName;
}

template <typename T> void PrintVec(const std::vector<T> &vec) {
  typename std::vector<T>::const_iterator it;
  std::cout << "{";
  for (it = vec.begin(); it != vec.end(); it++) {
    if (it != vec.begin())
      std::cout << ", ";
    std::cout << (*it);
  }
  std::cout << "}" << std::endl;
}

void SetStyle() {
  gStyle->SetEndErrorSize(6);
  gStyle->SetOptTitle(1);
  gStyle->SetOptStat(0);
}
