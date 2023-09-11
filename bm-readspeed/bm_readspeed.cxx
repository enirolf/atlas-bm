#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleDS.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleView.hxx>
#include <ROOT/RVec.hxx>

#include <TApplication.h>
#include <TBranch.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1F.h>
#include <TRootCanvas.h>
#include <TSystem.h>
#include <TTree.h>
#include <TTreePerfStats.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

using ROOT::RDataFrame;
using ROOT::Experimental::RNTuple;
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleReader;
using ROOT::Experimental::RNTupleView;

static void showHist(TH1D *hist) {
  auto app = TApplication("", nullptr, nullptr);
  auto c = new TCanvas("c", "", 700, 750);
  hist->DrawCopy();
  c->Modified();

  std::cout << "press ENTER to exit..." << std::endl;
  auto future = std::async(std::launch::async, getchar);
  while (true) {
    gSystem->ProcessEvents();
    if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      break;
  }
  return;
}

void bmRDFReadspeed(ROOT::RDataFrame &rdf, bool isNTuple) {
  const std::array<std::string_view, 8> containers = {"Electrons",
                                                      "Photons",
                                                      "TauJets",
                                                      "TauJets_MuonRM",
                                                      "DiTauJets",
                                                      "DiTauJetsLowPt",
                                                      "TauNeutralParticleFlowObjects",
                                                      "TauNeutralParticleFlowObjects_MuonRM"};
  const std::string sep = isNTuple ? "_" : ".";

  // Need to get access to the RInterface object.
  auto rdfEL = rdf.Define("NOOP", []() { return true; });
  std::vector<ROOT::RDF::RResultPtr<TH1D>> hists;

  for (const auto c : containers) {
    hists.emplace_back(rdfEL.Histo1D<ROOT::RVec<float>>({"hPt", "hPt", 128, 0, 20000}, std::string(c) + "AuxDyn" + sep + "pt"));
    hists.emplace_back(rdfEL.Histo1D<ROOT::RVec<float>>({"hEta", "hEta", 128, 0, 20000}, std::string(c) + "AuxDyn" + sep + "eta"));
    hists.emplace_back(rdfEL.Histo1D<ROOT::RVec<float>>({"hPhi", "hPhi", 128, 0, 20000}, std::string(c) + "AuxDyn" + sep + "phi"));
    hists.emplace_back(rdfEL.Histo1D<ROOT::RVec<float>>({"hM", "hM", 128, 0, 20000}, std::string(c) + "AuxDyn" + sep + "m"));
  }



  std::cout << "Events processed: " << *rdfEL.Count() << std::endl;
}

static void printUsage(std::string_view prog) {
  std::cout << prog << " [-v -h] -i INPUT_PATH -n STORE_NAME -s (ttree|rntuple)" << std::endl;
}

int main(int argc, char **argv) {
  std::string inputPath, storeName;
  bool isNTuple = false;

  int c;
  while ((c = getopt(argc, argv, "hi:n:s:")) != -1) {
    switch (c) {
    case 'h':
      printUsage(argv[0]);
      return 0;
    case 'i':
      inputPath = optarg;
      break;
    case 'n':
      storeName = optarg;
      break;
    case 's':
      if (strcmp(optarg, "rntuple") == 0) {
        isNTuple = true;
      } else if (strcmp(optarg, "ttree") != 0) {
        std::cerr << "Unknown mode: " << optarg << std::endl;
        return 1;
      }
      break;
    default:
      printUsage(argv[0]);
      return 1;
    }
  }

  if (inputPath == "") {
    std::cerr << "ERROR: please provide an input path\n" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  if (storeName == "") {
    std::cerr << "ERROR: please provide the name of the TTree/RNTuple to read\n" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  auto verbosity = ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(),
                                                           ROOT::Experimental::ELogLevel::kInfo);

  if (isNTuple) {
    ROOT::RDataFrame rdf = ROOT::RDF::Experimental::FromRNTuple(storeName, inputPath);
    bmRDFReadspeed(rdf, isNTuple);
  } else {
    auto file = TFile::Open(inputPath.c_str());
    auto tree = file->Get<TTree>(storeName.c_str());
    auto treeStats = new TTreePerfStats("ioperf", tree);
    ROOT::RDataFrame rdf(*tree);
    bmRDFReadspeed(rdf, isNTuple);
    treeStats->Print();
  }

  return 0;
}
