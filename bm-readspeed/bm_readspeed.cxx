#include <ROOT/RDataFrame.hxx>
#include <ROOT/RNTuple.hxx>
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
using namespace ROOT::VecOps;

const std::vector<std::string> kValidContainers = {"ElectronsAuxDyn",
                                                   "PhotonsAuxDyn",
                                                   "DiTauJetsAuxDyn",
                                                   "DiTauJetsLowPtAuxDyn",
                                                   "TauJetsAuxDyn",
                                                   "TauJets_MuonRMAuxDyn",
                                                   "TauNeutralParticleFlowObjectsAuxDyn",
                                                   "TauNeutralParticleFlowObjects_MuonRMAuxDyn"};

std::vector<std::string> getNContainers(int n) {
  std::vector<std::string> resContainers;
  std::sample(kValidContainers.begin(), kValidContainers.end(), std::back_inserter(resContainers),
              n, std::mt19937{std::random_device{}()});
  return resContainers;
}

struct BranchHistogram {
  TBranch *branch;
  std::vector<float> *branchEntry;
  std::unique_ptr<TH1F> histogram;
};

static void showHist(TH1F *hist) {
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

void bmNTupleReadspeed(std::string_view ntuplePath, std::string_view ntupleName,
                       const std::vector<std::string> &containers) {

  auto tInit = std::chrono::steady_clock::now();

  auto ntuple = RNTupleReader::Open(ntupleName, ntuplePath);

  auto viewElectrons_pt = ntuple->GetView<std::vector<float>>("ElectronsAuxDyn:pt");
  auto viewElectrons_eta = ntuple->GetView<std::vector<float>>("ElectronsAuxDyn:eta");
  auto viewElectrons_m = ntuple->GetView<std::vector<float>>("ElectronsAuxDyn:m");
  auto viewElectrons_phi = ntuple->GetView<std::vector<float>>("ElectronsAuxDyn:phi");

  auto histElectrons = new TH1F("Electrons", "Electrons", 128, 0, 2000000);

  auto viewPhotons_pt = ntuple->GetView<std::vector<float>>("PhotonsAuxDyn:pt");
  auto viewPhotons_eta = ntuple->GetView<std::vector<float>>("PhotonsAuxDyn:eta");
  auto viewPhotons_m = ntuple->GetView<std::vector<float>>("PhotonsAuxDyn:m");
  auto viewPhotons_phi = ntuple->GetView<std::vector<float>>("PhotonsAuxDyn:phi");

  auto histPhotons = new TH1F("Photons", "Photons", 128, 0, 2000000);

  auto viewDiTauJets_pt = ntuple->GetView<std::vector<float>>("DiTauJetsAuxDyn:pt");
  auto viewDiTauJets_eta = ntuple->GetView<std::vector<float>>("DiTauJetsAuxDyn:eta");
  auto viewDiTauJets_m = ntuple->GetView<std::vector<float>>("DiTauJetsAuxDyn:m");
  auto viewDiTauJets_phi = ntuple->GetView<std::vector<float>>("DiTauJetsAuxDyn:phi");

  auto histDiTauJets = new TH1F("DiTauJets", "DiTauJets", 128, 0, 2000000);

  auto viewTauJets_pt = ntuple->GetView<std::vector<float>>("TauJetsAuxDyn:pt");
  auto viewTauJets_eta = ntuple->GetView<std::vector<float>>("TauJetsAuxDyn:eta");
  auto viewTauJets_m = ntuple->GetView<std::vector<float>>("TauJetsAuxDyn:m");
  auto viewTauJets_phi = ntuple->GetView<std::vector<float>>("TauJetsAuxDyn:phi");

  auto histTauJets = new TH1F("TauJets", "TauJets", 128, 0, 2000000);

  auto viewDiTauJetsLowPt_pt = ntuple->GetView<std::vector<float>>("DiTauJetsLowPtAuxDyn:pt");
  auto viewDiTauJetsLowPt_eta = ntuple->GetView<std::vector<float>>("DiTauJetsLowPtAuxDyn:eta");
  auto viewDiTauJetsLowPt_m = ntuple->GetView<std::vector<float>>("DiTauJetsLowPtAuxDyn:m");
  auto viewDiTauJetsLowPt_phi = ntuple->GetView<std::vector<float>>("DiTauJetsLowPtAuxDyn:phi");

  auto histDiTauJetsLowPt = new TH1F("DiTauJetsLowPt", "DiTauJetsLowPt", 128, 0, 2000000);

  auto viewTauJets_MuonRM_pt = ntuple->GetView<std::vector<float>>("TauJets_MuonRMAuxDyn:pt");
  auto viewTauJets_MuonRM_eta = ntuple->GetView<std::vector<float>>("TauJets_MuonRMAuxDyn:eta");
  auto viewTauJets_MuonRM_m = ntuple->GetView<std::vector<float>>("TauJets_MuonRMAuxDyn:m");
  auto viewTauJets_MuonRM_phi = ntuple->GetView<std::vector<float>>("TauJets_MuonRMAuxDyn:phi");

  auto histTauJets_MuonRM = new TH1F("TauJets_MuonRM", "TauJets_MuonRM", 128, 0, 2000000);

  auto viewTauNeutralParticleFlowObjects_pt =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjectsAuxDyn:pt");
  auto viewTauNeutralParticleFlowObjects_eta =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjectsAuxDyn:eta");
  auto viewTauNeutralParticleFlowObjects_m =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjectsAuxDyn:m");
  auto viewTauNeutralParticleFlowObjects_phi =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjectsAuxDyn:phi");

  auto histTauNeutralParticleFlowObjects =
      new TH1F("TauNeutralParticleFlowObjects", "TauNeutralParticleFlowObjects", 128, 0, 2000000);

  auto viewTauNeutralParticleFlowObjects_MuonRM_pt =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjects_MuonRMAuxDyn:pt");
  auto viewTauNeutralParticleFlowObjects_MuonRM_eta =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjects_MuonRMAuxDyn:eta");
  auto viewTauNeutralParticleFlowObjects_MuonRM_m =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjects_MuonRMAuxDyn:m");
  auto viewTauNeutralParticleFlowObjects_MuonRM_phi =
      ntuple->GetView<std::vector<float>>("TauNeutralParticleFlowObjects_MuonRMAuxDyn:phi");

  auto histTauNeutralParticleFlowObjects_MuonRM =
      new TH1F("TauNeutralParticleFlowObjects_MuonRM", "TauNeutralParticleFlowObjects_MuonRM", 128,
               0, 2000000);

  std::chrono::steady_clock::time_point tStart;
  std::uint64_t nEvents = 0;

  for (auto e : ntuple->GetEntryRange()) {
    ++nEvents;
    if (nEvents == 1) {
      tStart = std::chrono::steady_clock::now();
    }

    auto invMassElectrons =
        InvariantMass(ROOT::RVecF(viewElectrons_pt(e)), ROOT::RVecF(viewElectrons_eta(e)),
                      ROOT::RVecF(viewElectrons_phi(e)), ROOT::RVecF(viewElectrons_m(e)));
    histElectrons->Fill(invMassElectrons);

    auto invMassPhotons =
        InvariantMass(ROOT::RVecF(viewPhotons_pt(e)), ROOT::RVecF(viewPhotons_eta(e)),
                      ROOT::RVecF(viewPhotons_phi(e)), ROOT::RVecF(viewPhotons_m(e)));
    histPhotons->Fill(invMassPhotons);

    auto invMassDiTauJets =
        InvariantMass(ROOT::RVecF(viewDiTauJets_pt(e)), ROOT::RVecF(viewDiTauJets_eta(e)),
                      ROOT::RVecF(viewDiTauJets_phi(e)), ROOT::RVecF(viewDiTauJets_m(e)));
    histDiTauJets->Fill(invMassDiTauJets);

    auto invMassDiTauJetsLowPt =
        InvariantMass(ROOT::RVecF(viewDiTauJetsLowPt_pt(e)), ROOT::RVecF(viewDiTauJetsLowPt_eta(e)),
                      ROOT::RVecF(viewDiTauJetsLowPt_phi(e)), ROOT::RVecF(viewDiTauJetsLowPt_m(e)));
    histDiTauJetsLowPt->Fill(invMassDiTauJetsLowPt);

    auto invMassTauJets =
        InvariantMass(ROOT::RVecF(viewTauJets_pt(e)), ROOT::RVecF(viewTauJets_eta(e)),
                      ROOT::RVecF(viewTauJets_phi(e)), ROOT::RVecF(viewTauJets_m(e)));
    histTauJets->Fill(invMassTauJets);

    auto invMassTauJets_MuonRM =
        InvariantMass(ROOT::RVecF(viewTauJets_MuonRM_pt(e)), ROOT::RVecF(viewTauJets_MuonRM_eta(e)),
                      ROOT::RVecF(viewTauJets_MuonRM_phi(e)), ROOT::RVecF(viewTauJets_MuonRM_m(e)));
    histTauJets_MuonRM->Fill(invMassTauJets_MuonRM);

    auto invMassTauNeutralParticleFlowObjects =
        InvariantMass(ROOT::RVecF(viewTauNeutralParticleFlowObjects_pt(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_eta(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_phi(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_m(e)));
    histTauNeutralParticleFlowObjects->Fill(invMassTauNeutralParticleFlowObjects);

    auto invMassTauNeutralParticleFlowObjects_MuonRM =
        InvariantMass(ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_pt(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_eta(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_phi(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_m(e)));
    histTauNeutralParticleFlowObjects_MuonRM->Fill(invMassTauNeutralParticleFlowObjects_MuonRM);

    if (e % 20000 == 0) {
      std::cout << e << " events processed" << std::endl;
    }
  }

  auto tEnd = std::chrono::steady_clock::now();
  auto tRuntimeInit = std::chrono::duration_cast<std::chrono::microseconds>(tStart - tInit).count();
  auto tRuntimeLoop = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart).count();

  std::cout << nEvents << "\t" << containers.size() * 4 << "\t" << tRuntimeInit << "\t"
            << tRuntimeLoop << std::endl;
}

void bmTreeReadspeed(std::string_view treePath, std::string_view treeName,
                     const std::vector<std::string> &containers) {
  auto tInit = std::chrono::steady_clock::now();

  auto file = std::unique_ptr<TFile>(TFile::Open(std::string(treePath).c_str()));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>(std::string(treeName).c_str()));

  std::vector<float> *Electrons_pt = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.pt", &Electrons_pt);
  std::vector<float> *Electrons_eta = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.eta", &Electrons_eta);
  std::vector<float> *Electrons_m = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.m", &Electrons_m);
  std::vector<float> *Electrons_phi = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.phi", &Electrons_phi);

  auto histElectrons = new TH1F("Electrons", "Electrons", 128, 0, 2000000);

  std::vector<float> *Photons_pt = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.pt", &Photons_pt);
  std::vector<float> *Photons_eta = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.eta", &Photons_eta);
  std::vector<float> *Photons_m = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.m", &Photons_m);
  std::vector<float> *Photons_phi = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.phi", &Photons_phi);

  auto histPhotons = new TH1F("Photons", "Photons", 128, 0, 2000000);

  std::vector<float> *DiTauJets_pt = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.pt", &DiTauJets_pt);
  std::vector<float> *DiTauJets_eta = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.eta", &DiTauJets_eta);
  std::vector<float> *DiTauJets_m = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.m", &DiTauJets_m);
  std::vector<float> *DiTauJets_phi = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.phi", &DiTauJets_phi);

  auto histDiTauJets = new TH1F("DiTauJets", "DiTauJets", 128, 0, 2000000);

  std::vector<float> *DiTauJetsLowPt_pt = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.pt", &DiTauJetsLowPt_pt);
  std::vector<float> *DiTauJetsLowPt_eta = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.eta", &DiTauJetsLowPt_eta);
  std::vector<float> *DiTauJetsLowPt_m = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.m", &DiTauJetsLowPt_m);
  std::vector<float> *DiTauJetsLowPt_phi = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.phi", &DiTauJetsLowPt_phi);

  auto histDiTauJetsLowPt = new TH1F("DiTauJetsLowPt", "DiTauJetsLowPt", 128, 0, 2000000);

  std::vector<float> *TauJets_pt = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.pt", &TauJets_pt);
  std::vector<float> *TauJets_eta = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.eta", &TauJets_eta);
  std::vector<float> *TauJets_m = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.m", &TauJets_m);
  std::vector<float> *TauJets_phi = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.phi", &TauJets_phi);

  auto histTauJets = new TH1F("TauJets", "TauJets", 128, 0, 2000000);

  std::vector<float> *TauJets_MuonRM_pt = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.pt", &TauJets_MuonRM_pt);
  std::vector<float> *TauJets_MuonRM_eta = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.eta", &TauJets_MuonRM_eta);
  std::vector<float> *TauJets_MuonRM_m = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.m", &TauJets_MuonRM_m);
  std::vector<float> *TauJets_MuonRM_phi = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.phi", &TauJets_MuonRM_phi);

  auto histTauJets_MuonRM = new TH1F("TauJets_MuonRM", "TauJets_MuonRM", 128, 0, 2000000);

  std::vector<float> *TauNeutralParticleFlowObjects_pt = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.pt",
                         &TauNeutralParticleFlowObjects_pt);
  std::vector<float> *TauNeutralParticleFlowObjects_eta = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.eta",
                         &TauNeutralParticleFlowObjects_eta);
  std::vector<float> *TauNeutralParticleFlowObjects_m = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.m", &TauNeutralParticleFlowObjects_m);
  std::vector<float> *TauNeutralParticleFlowObjects_phi = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.phi",
                         &TauNeutralParticleFlowObjects_phi);

  auto histTauNeutralParticleFlowObjects =
      new TH1F("TauNeutralParticleFlowObjects", "TauNeutralParticleFlowObjects", 128, 0, 2000000);

  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_pt = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.pt",
                         &TauNeutralParticleFlowObjects_MuonRM_pt);
  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_eta = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.eta",
                         &TauNeutralParticleFlowObjects_MuonRM_eta);
  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_m = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.m",
                         &TauNeutralParticleFlowObjects_MuonRM_m);
  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_phi = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.phi",
                         &TauNeutralParticleFlowObjects_MuonRM_phi);

  auto histTauNeutralParticleFlowObjects_MuonRM =
      new TH1F("TauNeutralParticleFlowObjects_MuonRM", "TauNeutralParticleFlowObjects_MuonRM", 128,
               0, 2000000);

  std::chrono::steady_clock::time_point tStart;
  std::uint64_t nEvents = 0;

  for (Int_t e = 0; e < tree->GetEntries(); ++e) {
    ++nEvents;
    if (nEvents == 1) {
      tStart = std::chrono::steady_clock::now();
    }

    tree->LoadTree(e);
    tree->GetEntry(e);

    auto invMassElectrons = InvariantMass(ROOT::RVecF(*Electrons_pt), ROOT::RVecF(*Electrons_eta),
                                          ROOT::RVecF(*Electrons_phi), ROOT::RVecF(*Electrons_m));
    histElectrons->Fill(invMassElectrons);

    auto invMassPhotons = InvariantMass(ROOT::RVecF(*Photons_pt), ROOT::RVecF(*Photons_eta),
                                        ROOT::RVecF(*Photons_phi), ROOT::RVecF(*Photons_m));
    histPhotons->Fill(invMassPhotons);

    auto invMassTauJets = InvariantMass(ROOT::RVecF(*TauJets_pt), ROOT::RVecF(*TauJets_eta),
                                        ROOT::RVecF(*TauJets_phi), ROOT::RVecF(*TauJets_m));
    histTauJets->Fill(invMassTauJets);

    auto invMassTauJets_MuonRM =
        InvariantMass(ROOT::RVecF(*TauJets_MuonRM_pt), ROOT::RVecF(*TauJets_MuonRM_eta),
                      ROOT::RVecF(*TauJets_MuonRM_phi), ROOT::RVecF(*TauJets_MuonRM_m));
    histTauJets_MuonRM->Fill(invMassTauJets_MuonRM);

    auto invMassDiTauJets = InvariantMass(ROOT::RVecF(*DiTauJets_pt), ROOT::RVecF(*DiTauJets_eta),
                                          ROOT::RVecF(*DiTauJets_phi), ROOT::RVecF(*DiTauJets_m));
    histDiTauJets->Fill(invMassDiTauJets);

    auto invMassDiTauJetsLowPt =
        InvariantMass(ROOT::RVecF(*DiTauJetsLowPt_pt), ROOT::RVecF(*DiTauJetsLowPt_eta),
                      ROOT::RVecF(*DiTauJetsLowPt_phi), ROOT::RVecF(*DiTauJetsLowPt_m));
    histDiTauJetsLowPt->Fill(invMassDiTauJetsLowPt);

    auto invMassTauNeutralParticleFlowObjects =
        InvariantMass(ROOT::RVecF(*TauNeutralParticleFlowObjects_pt),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_eta),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_phi),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_m));
    histTauNeutralParticleFlowObjects->Fill(invMassTauNeutralParticleFlowObjects);

    auto invMassTauNeutralParticleFlowObjects_MuonRM =
        InvariantMass(ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_pt),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_eta),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_phi),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_m));
    histTauNeutralParticleFlowObjects_MuonRM->Fill(invMassTauNeutralParticleFlowObjects_MuonRM);

    if (e % 20000 == 0) {
      std::cout << e << " events processed" << std::endl;
    }
  }

  auto tEnd = std::chrono::steady_clock::now();
  auto tRuntimeInit = std::chrono::duration_cast<std::chrono::microseconds>(tStart - tInit).count();
  auto tRuntimeLoop = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart).count();

  std::cout << nEvents << "\t" << containers.size() * 4 << "\t" << tRuntimeInit << "\t"
            << tRuntimeLoop << std::endl;
}

void bmRDFReadspeed(std::string_view storePath, std::string_view storeName, bool isNTuple,
                    const std::vector<std::string> &containers) {
  auto tInit = std::chrono::steady_clock::now();
  ROOT::RDataFrame rdf(storeName, storePath);

  std::chrono::steady_clock::time_point tStart;
  for (const auto &c : containers) {
    auto hPt = rdf.Histo1D(c + ":pt");
    auto hEta = rdf.Histo1D(c + ":eta");
    auto hMass = rdf.Histo1D(c + ":m");
    auto hPhi = rdf.Histo1D(c + ":phi");
  }
}

static void printUsage(std::string_view prog) {
  std::cout << prog << " -i INPUT_PATH -n STORE_NAME -m (ttree|rntuple) -c N_CONTAINERS"
            << std::endl;
}

int main(int argc, char **argv) {

  // Suppress (irrelevant) warnings
  gErrorIgnoreLevel = kError;

  std::string inputPath, storeName;
  bool checkNTuple = false;
  bool useRDF = false;
  std::vector<std::string> containers = kValidContainers;
  size_t nContainers = 0;

  int c;
  while ((c = getopt(argc, argv, "hi:n:m:c:")) != -1) {
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
    case 'm':
      if (strcmp(optarg, "rntuple") == 0) {
        checkNTuple = true;
      } else if (strcmp(optarg, "rdf+ttree") == 0) {
        useRDF = true;
      } else if (strcmp(optarg, "rdf+rntuple") == 0) {
        useRDF = true;
        checkNTuple = true;
      } else if (strcmp(optarg, "ttree") != 0) {
        std::cerr << "Unknown mode: " << optarg << std::endl;
        return 1;
      }
      break;
    case 'c':
      nContainers = std::atoi(optarg);
      if (nContainers > kValidContainers.size()) {
        std::cout << "Specified number of containers is higher than the number "
                     "of valid containers (8). Checking all valid containers..."
                  << std::endl;
      } else if (nContainers < 0) {
        std::cerr << "Invalid number of containers: " << nContainers << std::endl;
        return 1;
      } else {
        containers = getNContainers(nContainers);
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

  if (useRDF) {
    bmRDFReadspeed(inputPath, storeName, checkNTuple, containers);
  } else if (checkNTuple) {
    bmNTupleReadspeed(inputPath, storeName, containers);
  } else {
    bmTreeReadspeed(inputPath, storeName, containers);
  }

  return 0;
}