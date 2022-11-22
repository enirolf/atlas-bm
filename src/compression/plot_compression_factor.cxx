/**
 * Plot the compression factor and speed of TTree-based DAOD_PHYS/LITE files
 * for different compression algorithms.
 *
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include <ROOT/RDataFrame.hxx>
#include <TApplication.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TString.h>
#include <TSystem.h>

#include <future>
#include <iostream>
#include <map>

#include "util.hxx"

void plotLevelsPerAlg(ROOT::RDataFrame *rdf) {
  auto c = new TCanvas(
      "c_levels", "Compression factors per compression algorithm and level",
      constants::canvasWidth, constants::canvasHeight);
  c->Divide(2, 2, 0.0005, 0.005);
  Int_t currPad = 0;

  for (const auto &[cAlg, cName] : constants::compressionAlgorithms) {
    auto p = c->cd(++currPad);

    auto filteredRdf =
        rdf->Filter([cAlg](Int_t c) { return c == cAlg; }, {"compAlg"});

    auto plot = filteredRdf.Graph<Int_t, Float_t>("compLvl", "compFactor");

    setPlotStyle(plot.GetPtr());
    plot->SetTitle(cName.c_str());
    plot->GetXaxis()->SetTitle("Compression level");
    plot->GetYaxis()->SetTitle("Compression factor");
    plot->GetYaxis()->SetRangeUser(0., 8.);
    plot->DrawClone("AB");
  }
}

void plotAlgs(ROOT::RDataFrame *rdf) {
  auto c = new TCanvas("c_algs",
                       "Compression factors for different algorithms (with "
                       "optimal compression level)",
                       constants::canvasWidth, constants::canvasHeight);

  std::vector<Double_t> xs = {1., 2., 3., 4.};
  std::vector<Double_t> ys = {};
  std::vector<std::string> labels = {};

  for (const auto &[cAlg, cName] : constants::compressionAlgorithms) {
    auto compLvl =
        rdf->Filter([cAlg](Int_t c) { return c == cAlg; }, {"compAlg"})
            .Max("compFactor");

    ys.push_back(compLvl.GetValue());
    labels.push_back(cName);
  }

  TGraph *plot = new TGraph(xs.size(), xs.data(), ys.data());
  setPlotStyle(plot);

  plot->SetTitle("Compression factors for different algorithms (with "
                 "optimal compression level)");
  plot->GetXaxis()->SetTitle("Compression algorithm");
  plot->GetYaxis()->SetTitle("Compression factor");
  plot->GetYaxis()->SetRangeUser(0., 8.);

  for (Int_t i = 0; i < 4; ++i) {
    Int_t bin = plot->GetXaxis()->FindBin(i + 1.);
    plot->GetXaxis()->SetBinLabel(bin, labels[i].c_str());
  }

  plot->GetXaxis()->LabelsOption("h");
  plot->DrawClone("AB");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "No file specified, exiting..." << std::endl;
    return 1;
  }

  std::string inputPath = argv[1];

  new TApplication("", nullptr, nullptr);

  ROOT::RDataFrame compressionFactorDF =
      ROOT::RDataFrame("compressionDataTree", inputPath);

  plotLevelsPerAlg(&compressionFactorDF);
  plotAlgs(&compressionFactorDF);

  std::cout << "Press ENTER to exit..." << std::endl;
  auto future = std::async(std::launch::async, getchar);
  while (true) {
    gSystem->ProcessEvents();
    if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      break;
  }

  return 0;
}
