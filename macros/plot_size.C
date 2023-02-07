#include <iostream>
#include <string>

#include <TAxis.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TPaveLabel.h>
#include <TTree.h>

const int compressionSettings[] = {0, 207, 404, 505};

void plot_size(std::string resultsPath = "./results/size_full.txt") {
  std::ifstream resultsFile(resultsPath);

  if (!resultsFile.is_open()) {
    cout << "Failed to open " << resultsPath << endl;
    return 1;
  }

  std::string kind;
  int nEntries;
  uint64_t sizeOnDisk;
  uint64_t sizeInMemory;
  int compressionSetting;

  TGraph *treeSizeGraph = new TGraph();
  TGraph *ntupleSizeGraph = new TGraph();
  TGraph *compressionGraph = new TGraph();

  int step = 0;
  float maxSize = 0.;

  while (resultsFile >> kind >> nEntries >> sizeOnDisk >> sizeInMemory >>
         compressionSetting) {

    if (strcmp(kind.c_str(), "TTree") == 0) {
      treeSizeGraph->SetPoint(step, step + 0.5,
                              float(sizeOnDisk) / float(nEntries) / 1024.);
    } else {
      ntupleSizeGraph->SetPoint(step, step + 0.505,
                                float(sizeOnDisk) / float(nEntries) / 1024.);
    }

    step++;

    maxSize = std::max(maxSize, float(sizeInMemory) / float(nEntries) / 1024);
  }

  resultsFile.close();

  gStyle->SetBarWidth(1.07);

  treeSizeGraph->SetFillColor(kRed);
  ntupleSizeGraph->SetFillColor(kBlue);
  compressionGraph->SetFillColor(kGreen + 1);

  TMultiGraph *graphs = new TMultiGraph();
  graphs->Add(treeSizeGraph);
  graphs->Add(ntupleSizeGraph);

  auto c1 = new TCanvas();
  c1->SetWindowPosition(2800, 200);
  c1->SetTopMargin(0.1);
  c1->SetBottomMargin(0.1);
  c1->SetLeftMargin(0.1);
  c1->SetRightMargin(0.05);
  c1->SetGrid(1, 1);
  c1->cd();

  TH1F *axisHist1 = graphs->GetHistogram();
  axisHist1->SetMinimum(0.);
  axisHist1->SetMaximum(maxSize * 1.05);

  graphs->Draw("AB");

  auto legend = new TLegend(0.80, 0.89, 0.94, 0.80);
  legend->AddEntry(treeSizeGraph, "TTree", "f");
  legend->AddEntry(ntupleSizeGraph, "RNTuple", "f");
  legend->SetMargin(0.4);
  legend->Draw();

  gStyle->SetOptTitle(0);
  TPaveLabel *title = new TPaveLabel(
      0., maxSize + 10, 8., maxSize + 30,
      "On-disk event size of TTree vs. RNTuple-based DAOD_PHYS files", "nbndc");
  title->SetFillColor(gStyle->GetTitleFillColor());
  title->Draw();

  axisHist1->GetXaxis()->SetNdivisions(4);
  axisHist1->GetXaxis()->SetRangeUser(0.05, 10.);

  const std::string labels[] = {"uncompr.", "lzma", "lz4", "zstd"};

  // Manually draw labels, just to make sure they center nicely between both
  // bins.
  for (size_t i = 0; i < 4; i++) {
    axisHist1->GetXaxis()->ChangeLabel(i, -1., 0, -1, -1, -1, "");
    TPaveLabel *label =
        new TPaveLabel(i * 2, 0, (i * 2) + 2, -8, labels[i].c_str(), "nbndc");
    label->SetFillColor(gStyle->GetTitleFillColor());
    label->SetTextFont(42);
    label->Draw();
  }

  axisHist1->GetYaxis()->SetTitle("Average event size [kB]");

  c1->SaveAs("figures/PHYS_DAOD_FULL_event_size.pdf");
}
