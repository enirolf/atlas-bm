#include "../bm-utils/plot_util.C"

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ROOT/RVec.hxx>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <TPad.h>

using namespace ROOT::VecOps;

const bool kRemoveOutliers = false;

struct ReadSpeedData {
  TGraphErrors *eventThroughputGraph;
  float eventThroughputMean, eventThroughputErr;

  // TGraphErrors *byteThroughputGraph;
  // float byteThroughputMean, byteThroughputErr;
};

void drawGraph(std::map<std::string, ReadSpeedData> &readSpeedData,
               float maxThroughput, std::string_view physFileType, std::string_view medium,
               bool save = false) {
  //==========================================================================//
  // EVENT THROUGHPUT                                                         //
  //==========================================================================//

  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//

  TCanvas *canvas = new TCanvas(Form("canvas_bulk_throughput_%s", std::string(medium).c_str()),
                                Form("canvas_bulk_throughput_%s", std::string(medium).c_str()),
                                1200);
  canvas->SetFillStyle(4000);
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto pad = new TPad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad->SetTopMargin(0.085);
  pad->SetBottomMargin(0.08);
  pad->SetLeftMargin(0.12);
  pad->SetRightMargin(0.02);
  pad->SetFillStyle(4000);
  pad->SetFrameFillStyle(4000);
  pad->Draw();
  canvas->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//
  int nBins = 4;
  int binStart = 2;
  int binInterval = 2;

  TH1F *helper = new TH1F("", "", nBins, -0.25, nBins + 0.25);
  helper->SetTitle("DAOD_PHYS event throughput, SSD");
  // helper->SetTitleOffset(0.5);
  helper->GetXaxis()->SetTickSize(0);
  // helper->GetXaxis()->SetNdivisions(nBins * 2);
  helper->GetXaxis()->SetLabelOffset(0.025);
  // helper->GetYaxis()->SetMaxDigits(3);
  helper->GetYaxis()->SetTickSize(0.01);
  helper->GetYaxis()->SetLabelSize(0.05);
  helper->GetYaxis()->SetTitle("Events / s");
  helper->GetYaxis()->SetTitleSize(0.05);
  helper->GetYaxis()->SetTitleOffset(1.1);
  helper->SetMinimum(0);
  helper->SetMaximum(maxThroughput);

  float labelSize = 0.0375;

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "TTree");
    } else if (i == binStart + binInterval) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "TTree (bulk IO)");
    } else if (i == binStart + (2 * binInterval)) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "RNTuple");
    } else if (i == binStart + (3 * binInterval)) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "RNTuple (bulk IO)");
    } else {
      helper->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  pad->cd();
  gPad->SetGridy();

  helper->Draw();

  for (const auto &[setting, data] : readSpeedData) {
    auto graph = data.eventThroughputGraph;

    graph->SetLineColor(12);
    graph->SetMarkerColor(12);
    graph->SetFillColor(colors.at(setting));
    // graph->SetFillStyle(styles.at(setting));
    graph->SetLineWidth(2);
    graph->Draw("B1");
    graph->Draw("P");

    for (int i = 0; i < graph->GetN(); ++i) {

      double x, y;
      graph->GetPoint(i, x, y);

      if (y < 0)
        continue;

      y /= readSpeedData["ttree"].eventThroughputMean;

      std::ostringstream val;
      val.precision(2);
      val << "#times" << std::fixed << y;

      TLatex tval;
      tval.SetTextColor(kWhite);
      tval.SetTextSize(0.04);
      tval.SetTextAlign(21);
      tval.DrawLatex(x, maxThroughput * 0.025, val.str().c_str());
    }
  }


  TText l;
  l.SetTextSize(0.025);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.915, 0.9, "95% CL");

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvas->Print(Form("figures/readspeed_bulk_throughput_%s_%s.pdf", std::string(medium).c_str(), std::string(physFileType).c_str()));
    canvas->Print(Form("figures/readspeed_bulk_throughput_%s_%s.png", std::string(medium).c_str(), std::string(physFileType).c_str()));
  }
}

void plot(std::string_view resultsPathBase, std::string_view medium,
          std::string_view physFileType = "data", bool save = false) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//
  float nEvents = (physFileType == "data" ? 209062 : 180000); // TODO add to results file
  float eventLoopTime;

  // storage setting -> readspeed data
  std::map<std::string, ReadSpeedData> readSpeedData;

  auto storageSettings = std::vector<std::string>{"ttree", "ttree_bulk", "rntuple_uring", "rntuple_uring_bulk"};

  for (const std::string setting : storageSettings) {
    std::string resultsFilePath = std::string(resultsPathBase) + "/" + setting + ".data";

    std::cout << "** Reading from " << resultsFilePath << std::endl;

    std::ifstream resultsFile(resultsFilePath);

    std::vector<float> eventLoopTimes;

    while (resultsFile >> eventLoopTime) {
      eventLoopTimes.push_back(eventLoopTime);
    }

    std::cout << "* Event throughput" << std::endl;

    // float eventLoopTimeMedian = TMath::Median(eventLoopTimes.size(), eventLoopTimes.data());

    RVec<float> eventLoopTimeVec(eventLoopTimes.begin(), eventLoopTimes.end());
    // auto filteredEventLoopTimes = Filter(eventLoopTimeVec, [eventLoopTimeMedian](float loopTime) {
    //   return loopTime <= eventLoopTimeMedian + (eventLoopTimeMedian * 0.5) &&
    //          loopTime >= eventLoopTimeMedian - (eventLoopTimeMedian * 0.5);
    // });

    // std::cout << "\tRemoved " << eventLoopTimeVec.size() - filteredEventLoopTimes.size() << " (of "
    //           << eventLoopTimeVec.size() << " total measurements) throughput outliers" << std::endl;

    float eventLoopTimeMean = Mean(eventLoopTimeVec);
    float eventLoopTimeErr = StdErr(eventLoopTimeVec);
    float eventThroughputMean = nEvents / eventLoopTimeMean;
    float eventThroughputMax = nEvents / (eventLoopTimeMean - eventLoopTimeErr);
    float eventThroughputMin = nEvents / (eventLoopTimeMean + eventLoopTimeErr);
    float eventThroughputErr = (eventThroughputMax - eventThroughputMin) / 2.;

    std::cout << "\tMean = " << eventThroughputMean << "\tError = " << eventThroughputErr
              << std::endl;

    readSpeedData[setting] =
        ReadSpeedData{new TGraphErrors(), eventThroughputMean, eventThroughputErr};
  }

  // readSpeedData["filler"] =
  //     ReadSpeedData{new TGraphErrors(), 0., 0., new TGraphErrors(), 0., 0.};

  float maxEventThroughput = 0.;
  for (const auto &[setting, data] : readSpeedData) {
    float x = 0.5;
    if (setting == "ttree")
      x += 0;
    else if (setting == "ttree_bulk")
      x += 1;
    else if (setting == "rntuple_uring")
      x += 2;
    else if (setting == "rntuple_uring_bulk")
      x += 3;

    auto gEvent = data.eventThroughputGraph;
    gEvent->SetPoint(0, x + 0, data.eventThroughputMean);
    gEvent->SetPoint(1, x + 1.25, -1);
    gEvent->SetPointError(0, 0, data.eventThroughputErr);
    maxEventThroughput =
        std::max(maxEventThroughput, data.eventThroughputMean + data.eventThroughputErr);
  }

  drawGraph(readSpeedData, maxEventThroughput * 1.1, physFileType, medium, save);
}

void plot_bulk() {
  SetStyle();

  plot("results/zstd_bulk_throughput", "SSD", "data", true);
}
