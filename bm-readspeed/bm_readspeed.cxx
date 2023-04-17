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
// using namespace ROOT::VecOps;

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
                       bool verbose = false) {

  auto tInit = std::chrono::steady_clock::now();

  auto ntuple = RNTupleReader::Open(ntupleName, ntuplePath);
  if (verbose)
    ntuple->EnableMetrics();

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
                      ROOT::RVecF(viewElectrons_m(e)), ROOT::RVecF(viewElectrons_phi(e)));
    histElectrons->Fill(invMassElectrons);

    auto invMassPhotons =
        InvariantMass(ROOT::RVecF(viewPhotons_pt(e)), ROOT::RVecF(viewPhotons_eta(e)),
                      ROOT::RVecF(viewPhotons_m(e)), ROOT::RVecF(viewPhotons_phi(e)));
    histPhotons->Fill(invMassPhotons);

    auto invMassDiTauJets =
        InvariantMass(ROOT::RVecF(viewDiTauJets_pt(e)), ROOT::RVecF(viewDiTauJets_eta(e)),
                      ROOT::RVecF(viewDiTauJets_m(e)), ROOT::RVecF(viewDiTauJets_phi(e)));
    histDiTauJets->Fill(invMassDiTauJets);

    auto invMassDiTauJetsLowPt =
        InvariantMass(ROOT::RVecF(viewDiTauJetsLowPt_pt(e)), ROOT::RVecF(viewDiTauJetsLowPt_eta(e)),
                      ROOT::RVecF(viewDiTauJetsLowPt_m(e)), ROOT::RVecF(viewDiTauJetsLowPt_phi(e)));
    histDiTauJetsLowPt->Fill(invMassDiTauJetsLowPt);

    auto invMassTauJets =
        InvariantMass(ROOT::RVecF(viewTauJets_pt(e)), ROOT::RVecF(viewTauJets_eta(e)),
                      ROOT::RVecF(viewTauJets_m(e)), ROOT::RVecF(viewTauJets_phi(e)));
    histTauJets->Fill(invMassTauJets);

    auto invMassTauJets_MuonRM =
        InvariantMass(ROOT::RVecF(viewTauJets_MuonRM_pt(e)), ROOT::RVecF(viewTauJets_MuonRM_eta(e)),
                      ROOT::RVecF(viewTauJets_MuonRM_m(e)), ROOT::RVecF(viewTauJets_MuonRM_phi(e)));
    histTauJets_MuonRM->Fill(invMassTauJets_MuonRM);

    auto invMassTauNeutralParticleFlowObjects =
        InvariantMass(ROOT::RVecF(viewTauNeutralParticleFlowObjects_pt(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_eta(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_m(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_phi(e)));
    histTauNeutralParticleFlowObjects->Fill(invMassTauNeutralParticleFlowObjects);

    auto invMassTauNeutralParticleFlowObjects_MuonRM =
        InvariantMass(ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_pt(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_eta(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_m(e)),
                      ROOT::RVecF(viewTauNeutralParticleFlowObjects_MuonRM_phi(e)));
    histTauNeutralParticleFlowObjects_MuonRM->Fill(invMassTauNeutralParticleFlowObjects_MuonRM);
  }

  auto tEnd = std::chrono::steady_clock::now();
  auto tRuntimeInit = std::chrono::duration_cast<std::chrono::microseconds>(tStart - tInit).count();
  auto tRuntimeLoop = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart).count();

  std::cout << nEvents << "\t" << tRuntimeInit << "\t" << tRuntimeLoop << std::endl;
  if (verbose)
    ntuple->PrintInfo(ROOT::Experimental::ENTupleInfo::kMetrics);
}

void bmTreeReadspeed(std::string_view treePath, std::string_view treeName, bool verbose = false) {
  auto tInit = std::chrono::steady_clock::now();

  auto file = std::unique_ptr<TFile>(TFile::Open(std::string(treePath).c_str()));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>(std::string(treeName).c_str()));

  TTreePerfStats *ps = new TTreePerfStats("ioperf", tree.get());

  std::vector<float> *Electrons_pt = nullptr;
  TBranch *brElectrons_pt = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.pt", &Electrons_pt, &brElectrons_pt);
  std::vector<float> *Electrons_eta = nullptr;
  TBranch *brElectrons_eta = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.eta", &Electrons_eta, &brElectrons_eta);
  std::vector<float> *Electrons_m = nullptr;
  TBranch *brElectrons_m = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.m", &Electrons_m, &brElectrons_m);
  std::vector<float> *Electrons_phi = nullptr;
  TBranch *brElectrons_phi = nullptr;
  tree->SetBranchAddress("ElectronsAuxDyn.phi", &Electrons_phi, &brElectrons_phi);

  auto histElectrons = new TH1F("Electrons", "Electrons", 128, 0, 2000000);

  std::vector<float> *Photons_pt = nullptr;
  TBranch *brPhotons_pt = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.pt", &Photons_pt, &brPhotons_pt);
  std::vector<float> *Photons_eta = nullptr;
  TBranch *brPhotons_eta = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.eta", &Photons_eta, &brPhotons_eta);
  std::vector<float> *Photons_m = nullptr;
  TBranch *brPhotons_m = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.m", &Photons_m, &brPhotons_m);
  std::vector<float> *Photons_phi = nullptr;
  TBranch *brPhotons_phi = nullptr;
  tree->SetBranchAddress("PhotonsAuxDyn.phi", &Photons_phi, &brPhotons_phi);

  auto histPhotons = new TH1F("Photons", "Photons", 128, 0, 2000000);

  std::vector<float> *DiTauJets_pt = nullptr;
  TBranch *brDiTauJets_pt = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.pt", &DiTauJets_pt, &brDiTauJets_pt);
  std::vector<float> *DiTauJets_eta = nullptr;
  TBranch *brDiTauJets_eta = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.eta", &DiTauJets_eta, &brDiTauJets_eta);
  std::vector<float> *DiTauJets_m = nullptr;
  TBranch *brDiTauJets_m = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.m", &DiTauJets_m, &brDiTauJets_m);
  std::vector<float> *DiTauJets_phi = nullptr;
  TBranch *brDiTauJets_phi = nullptr;
  tree->SetBranchAddress("DiTauJetsAuxDyn.phi", &DiTauJets_phi, &brDiTauJets_phi);

  auto histDiTauJets = new TH1F("DiTauJets", "DiTauJets", 128, 0, 2000000);

  std::vector<float> *DiTauJetsLowPt_pt = nullptr;
  TBranch *brDiTauJetsLowPt_pt = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.pt", &DiTauJetsLowPt_pt, &brDiTauJetsLowPt_pt);
  std::vector<float> *DiTauJetsLowPt_eta = nullptr;
  TBranch *brDiTauJetsLowPt_eta = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.eta", &DiTauJetsLowPt_eta, &brDiTauJetsLowPt_eta);
  std::vector<float> *DiTauJetsLowPt_m = nullptr;
  TBranch *brDiTauJetsLowPt_m = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.m", &DiTauJetsLowPt_m, &brDiTauJetsLowPt_m);
  std::vector<float> *DiTauJetsLowPt_phi = nullptr;
  TBranch *brDiTauJetsLowPt_phi = nullptr;
  tree->SetBranchAddress("DiTauJetsLowPtAuxDyn.phi", &DiTauJetsLowPt_phi, &brDiTauJetsLowPt_phi);

  auto histDiTauJetsLowPt = new TH1F("DiTauJetsLowPt", "DiTauJetsLowPt", 128, 0, 2000000);

  std::vector<float> *TauJets_pt = nullptr;
  TBranch *brTauJets_pt = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.pt", &TauJets_pt, &brTauJets_pt);
  std::vector<float> *TauJets_eta = nullptr;
  TBranch *brTauJets_eta = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.eta", &TauJets_eta, &brTauJets_eta);
  std::vector<float> *TauJets_m = nullptr;
  TBranch *brTauJets_m = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.m", &TauJets_m, &brTauJets_m);
  std::vector<float> *TauJets_phi = nullptr;
  TBranch *brTauJets_phi = nullptr;
  tree->SetBranchAddress("TauJetsAuxDyn.phi", &TauJets_phi, &brTauJets_phi);

  auto histTauJets = new TH1F("TauJets", "TauJets", 128, 0, 2000000);

  std::vector<float> *TauJets_MuonRM_pt = nullptr;
  TBranch *brTauJets_MuonRM_pt = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.pt", &TauJets_MuonRM_pt, &brTauJets_MuonRM_pt);
  std::vector<float> *TauJets_MuonRM_eta = nullptr;
  TBranch *brTauJets_MuonRM_eta = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.eta", &TauJets_MuonRM_eta, &brTauJets_MuonRM_eta);
  std::vector<float> *TauJets_MuonRM_m = nullptr;
  TBranch *brTauJets_MuonRM_m = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.m", &TauJets_MuonRM_m, &brTauJets_MuonRM_m);
  std::vector<float> *TauJets_MuonRM_phi = nullptr;
  TBranch *brTauJets_MuonRM_phi = nullptr;
  tree->SetBranchAddress("TauJets_MuonRMAuxDyn.phi", &TauJets_MuonRM_phi, &brTauJets_MuonRM_phi);

  auto histTauJets_MuonRM = new TH1F("TauJets_MuonRM", "TauJets_MuonRM", 128, 0, 2000000);

  std::vector<float> *TauNeutralParticleFlowObjects_pt = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_pt = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.pt",
                         &TauNeutralParticleFlowObjects_pt, &brTauNeutralParticleFlowObjects_pt);
  std::vector<float> *TauNeutralParticleFlowObjects_eta = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_eta = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.eta",
                         &TauNeutralParticleFlowObjects_eta, &brTauNeutralParticleFlowObjects_eta);
  std::vector<float> *TauNeutralParticleFlowObjects_m = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_m = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.m", &TauNeutralParticleFlowObjects_m,
                         &brTauNeutralParticleFlowObjects_m);
  std::vector<float> *TauNeutralParticleFlowObjects_phi = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_phi = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjectsAuxDyn.phi",
                         &TauNeutralParticleFlowObjects_phi, &brTauNeutralParticleFlowObjects_phi);

  auto histTauNeutralParticleFlowObjects =
      new TH1F("TauNeutralParticleFlowObjects", "TauNeutralParticleFlowObjects", 128, 0, 2000000);

  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_pt = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_MuonRM_pt = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.pt",
                         &TauNeutralParticleFlowObjects_MuonRM_pt,
                         &brTauNeutralParticleFlowObjects_MuonRM_pt);
  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_eta = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_MuonRM_eta = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.eta",
                         &TauNeutralParticleFlowObjects_MuonRM_eta,
                         &brTauNeutralParticleFlowObjects_MuonRM_eta);
  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_m = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_MuonRM_m = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.m",
                         &TauNeutralParticleFlowObjects_MuonRM_m,
                         &brTauNeutralParticleFlowObjects_MuonRM_m);
  std::vector<float> *TauNeutralParticleFlowObjects_MuonRM_phi = nullptr;
  TBranch *brTauNeutralParticleFlowObjects_MuonRM_phi = nullptr;
  tree->SetBranchAddress("TauNeutralParticleFlowObjects_MuonRMAuxDyn.phi",
                         &TauNeutralParticleFlowObjects_MuonRM_phi,
                         &brTauNeutralParticleFlowObjects_MuonRM_phi);

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

    brElectrons_pt->GetEntry(e);
    brElectrons_eta->GetEntry(e);
    brElectrons_m->GetEntry(e);
    brElectrons_phi->GetEntry(e);

    auto invMassElectrons = InvariantMass(ROOT::RVecF(*Electrons_pt), ROOT::RVecF(*Electrons_eta),
                                          ROOT::RVecF(*Electrons_m), ROOT::RVecF(*Electrons_phi));
    histElectrons->Fill(invMassElectrons);

    brPhotons_pt->GetEntry(e);
    brPhotons_eta->GetEntry(e);
    brPhotons_m->GetEntry(e);
    brPhotons_phi->GetEntry(e);

    auto invMassPhotons = InvariantMass(ROOT::RVecF(*Photons_pt), ROOT::RVecF(*Photons_eta),
                                        ROOT::RVecF(*Photons_m), ROOT::RVecF(*Photons_phi));
    histPhotons->Fill(invMassPhotons);

    brTauJets_pt->GetEntry(e);
    brTauJets_eta->GetEntry(e);
    brTauJets_m->GetEntry(e);
    brTauJets_phi->GetEntry(e);

    auto invMassTauJets = InvariantMass(ROOT::RVecF(*TauJets_pt), ROOT::RVecF(*TauJets_eta),
                                        ROOT::RVecF(*TauJets_m), ROOT::RVecF(*TauJets_phi));
    histTauJets->Fill(invMassTauJets);

    brTauJets_MuonRM_pt->GetEntry(e);
    brTauJets_MuonRM_eta->GetEntry(e);
    brTauJets_MuonRM_m->GetEntry(e);
    brTauJets_MuonRM_phi->GetEntry(e);

    auto invMassTauJets_MuonRM =
        InvariantMass(ROOT::RVecF(*TauJets_MuonRM_pt), ROOT::RVecF(*TauJets_MuonRM_eta),
                      ROOT::RVecF(*TauJets_MuonRM_m), ROOT::RVecF(*TauJets_MuonRM_phi));
    histTauJets_MuonRM->Fill(invMassTauJets_MuonRM);

    brDiTauJets_pt->GetEntry(e);
    brDiTauJets_eta->GetEntry(e);
    brDiTauJets_m->GetEntry(e);
    brDiTauJets_phi->GetEntry(e);

    auto invMassDiTauJets = InvariantMass(ROOT::RVecF(*DiTauJets_pt), ROOT::RVecF(*DiTauJets_eta),
                                          ROOT::RVecF(*DiTauJets_m), ROOT::RVecF(*DiTauJets_phi));
    histDiTauJets->Fill(invMassDiTauJets);

    brDiTauJetsLowPt_pt->GetEntry(e);
    brDiTauJetsLowPt_eta->GetEntry(e);
    brDiTauJetsLowPt_m->GetEntry(e);
    brDiTauJetsLowPt_phi->GetEntry(e);

    auto invMassDiTauJetsLowPt =
        InvariantMass(ROOT::RVecF(*DiTauJetsLowPt_pt), ROOT::RVecF(*DiTauJetsLowPt_eta),
                      ROOT::RVecF(*DiTauJetsLowPt_m), ROOT::RVecF(*DiTauJetsLowPt_phi));
    histDiTauJetsLowPt->Fill(invMassDiTauJetsLowPt);

    brTauNeutralParticleFlowObjects_pt->GetEntry(e);
    brTauNeutralParticleFlowObjects_eta->GetEntry(e);
    brTauNeutralParticleFlowObjects_m->GetEntry(e);
    brTauNeutralParticleFlowObjects_phi->GetEntry(e);

    auto invMassTauNeutralParticleFlowObjects =
        InvariantMass(ROOT::RVecF(*TauNeutralParticleFlowObjects_pt),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_eta),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_m),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_phi));
    histTauNeutralParticleFlowObjects->Fill(invMassTauNeutralParticleFlowObjects);

    brTauNeutralParticleFlowObjects_MuonRM_pt->GetEntry(e);
    brTauNeutralParticleFlowObjects_MuonRM_eta->GetEntry(e);
    brTauNeutralParticleFlowObjects_MuonRM_m->GetEntry(e);
    brTauNeutralParticleFlowObjects_MuonRM_phi->GetEntry(e);

    auto invMassTauNeutralParticleFlowObjects_MuonRM =
        InvariantMass(ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_pt),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_eta),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_m),
                      ROOT::RVecF(*TauNeutralParticleFlowObjects_MuonRM_phi));
    histTauNeutralParticleFlowObjects_MuonRM->Fill(invMassTauNeutralParticleFlowObjects_MuonRM);
  }

  auto tEnd = std::chrono::steady_clock::now();
  auto tRuntimeInit = std::chrono::duration_cast<std::chrono::microseconds>(tStart - tInit).count();
  auto tRuntimeLoop = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart).count();

  std::cout << nEvents << "\t" << tRuntimeInit << "\t" << tRuntimeLoop << std::endl;
  if (verbose)
    ps->Print();
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
  const std::string sep = isNTuple ? ":" : ".";
  auto tInit = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point tsFirst;
  bool tsFirstSet = false;

  auto rdfTiming = rdf.Define("TIMING",
                              [&tsFirst, &tsFirstSet]() {
                                if (!tsFirstSet)
                                  tsFirst = std::chrono::steady_clock::now();
                                tsFirstSet = true;
                                return tsFirstSet;
                              })
                       .Filter([](bool b) { return b; }, {"TIMING"});

  for (const auto c : containers) {
    rdfTiming = rdfTiming.Define(
        "invMass" + std::string(c), ROOT::VecOps::InvariantMass<float>,
        {std::string(c) + "AuxDyn" + sep + "pt", std::string(c) + "AuxDyn" + sep + "eta",
         std::string(c) + "AuxDyn" + sep + "m", std::string(c) + "AuxDyn" + sep + "phi"});
  }

  auto tStart = std::chrono::steady_clock::now();

  for (const auto c : containers) {
    auto histInvMass = rdfTiming.Histo1D<float>({"histInvMas", "histInvMas", 128, 0, 2000000},
                                                "invMass" + std::string(c));
    *histInvMass;
  }

  auto tEnd = std::chrono::steady_clock::now();
  auto tRuntimeInit = std::chrono::duration_cast<std::chrono::microseconds>(tStart - tInit).count();
  auto tRuntimeLoop = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart).count();

  std::cout << " " << tRuntimeInit << " " << tRuntimeLoop << std::endl;
}

static void printUsage(std::string_view prog) {
  std::cout << prog << " [-v -m -r -h] -i INPUT_PATH -n STORE_NAME -s (ttree|rntuple)" << std::endl;
}

int main(int argc, char **argv) {

  // Suppress (irrelevant) warnings
  gErrorIgnoreLevel = kError;

  std::string inputPath, storeName;
  bool isNTuple = false;
  bool useRDF = false;
  bool verbose = false;

  int c;
  while ((c = getopt(argc, argv, "hmvri:n:s:")) != -1) {
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
    case 'm':
      ROOT::EnableImplicitMT();
      break;
    case 'v':
      verbose = true;
      break;
    case 'r':
      useRDF = true;
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
    if (isNTuple) {
      ROOT::RDataFrame rdf = ROOT::RDF::Experimental::FromRNTuple(storeName, inputPath);
      bmRDFReadspeed(rdf, isNTuple);
    } else {
      ROOT::RDataFrame rdf(storeName, inputPath);
      bmRDFReadspeed(rdf, isNTuple);
    }
  } else if (isNTuple) {
    bmNTupleReadspeed(inputPath, storeName, verbose);
  } else {
    bmTreeReadspeed(inputPath, storeName, verbose);
  }

  return 0;
}
