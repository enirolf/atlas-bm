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

  TGraphErrors *byteThroughputGraph;
  float byteThroughputMean, byteThroughputErr;
};

void drawGraph(std::map<int, std::map<std::string, ReadSpeedData>> &readSpeedData,
               float maxThroughput, std::string_view physFileType, std::string_view medium,
               bool withUring, std::string_view plotKind, bool save = false) {
  //==========================================================================//
  // EVENT THROUGHPUT                                                         //
  //==========================================================================//

  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//

  TCanvas *canvas = new TCanvas(
      Form("canvas_%s_throughput_%s", std::string(plotKind).c_str(), std::string(medium).c_str()),
      Form("canvas_%s_throughput_%s", std::string(plotKind).c_str(), std::string(medium).c_str()),
      1200, 1000);
  canvas->SetFillStyle(4000);
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto pad = new TPad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad->SetTopMargin(0.055);
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
  int nBins = withUring ? 12 : 9;
  int binStart = nBins / 3 + 1;
  int binInterval = nBins / 2 + 2;

  TH1F *helper = new TH1F("", "", nBins, 0, nBins);
  helper->GetXaxis()->SetTickSize(0);
  helper->GetXaxis()->SetNdivisions(nBins * 2);
  helper->GetXaxis()->SetLabelOffset(0.025);
  helper->GetYaxis()->SetMaxDigits(3);
  helper->GetYaxis()->SetTickSize(0.01);
  helper->GetYaxis()->SetLabelSize(0.05);
  if (plotKind == "event")
    helper->GetYaxis()->SetTitle("Events / s");
  else
    helper->GetYaxis()->SetTitle("MB / s");
  helper->GetYaxis()->SetTitleSize(0.05);
  helper->GetYaxis()->SetTitleOffset(1.1);
  helper->SetMinimum(0);
  helper->SetMaximum(maxThroughput);

  float labelSize = 0.05;

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "zstd");
    } else if (i == binStart + binInterval) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "lzma (level 1)");
    } else if (i == binStart + (2 * binInterval)) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "lzma (level 7)");
    } else {
      helper->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  pad->cd();
  gPad->SetGridy();

  helper->Draw();

  for (const auto &[compression, formats] : readSpeedData) {
    for (const auto &[format, data] : formats) {
      auto graph = plotKind == "event" ? data.eventThroughputGraph : data.byteThroughputGraph;

      graph->SetLineColor(12);
      graph->SetMarkerColor(12);
      graph->SetFillColor(colors.at(format));
      graph->SetFillStyle(styles.at(format));
      graph->SetLineWidth(2);
      graph->Draw("B1");
      graph->Draw("P");

      if (format == "rntuple" || format == "rntuple_uring") {
        for (int i = 0; i < graph->GetN(); ++i) {

          double x, y;
          graph->GetPoint(i, x, y);

          if (y < 0)
            continue;

          if (plotKind == "event")
            y /= readSpeedData[compression]["ttree"].eventThroughputMean;
          else
            y /= readSpeedData[compression]["ttree"].byteThroughputMean;

          std::ostringstream val;
          val.precision(1);
          val << "#times" << std::fixed << y;

          TLatex tval;
          tval.SetTextColor(kWhite);
          if (withUring)
            tval.SetTextSize(0.04);
          else
            tval.SetTextSize(0.05);
          tval.SetTextAlign(21);
          tval.DrawLatex(x, maxThroughput * 0.025, val.str().c_str());
        }
      }
    }
  }

  for (unsigned i = 0; i < nBins; i += (binInterval / 2)) {
    TLine *line = new TLine(i, 0, i, maxThroughput);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *leg = new TLegend(0.6, 0.8, 0.97, 0.935);
  leg->AddEntry(readSpeedData[505]["ttree"].eventThroughputGraph, "TTree", "F");
  leg->AddEntry(readSpeedData[505]["rntuple"].eventThroughputGraph, "RNTuple", "F");
  if (withUring) {
    leg->AddEntry(readSpeedData[505]["rntuple_uring"].eventThroughputGraph, "RNTuple (w/ io_uring)",
                  "F");
  }
  leg->SetNColumns(1);
  leg->Draw();

  TText l;
  l.SetTextSize(0.025);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.9, 0.785, "95% CL");

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvas->Print(Form("figures/readspeed_%s_throughput_%s_%s.pdf", std::string(plotKind).c_str(),
                       std::string(medium).c_str(), std::string(physFileType).c_str()));
    canvas->Print(Form("figures/readspeed_%s_throughput_%s_%s.png", std::string(plotKind).c_str(),
                       std::string(medium).c_str(), std::string(physFileType).c_str()));
  }
}

void plot(std::string_view resultsPathBase, std::string_view medium,
          std::string_view physFileType = "data", bool withUring = true, bool save = false) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//
  float nEvents = (physFileType == "data" ? 209062 : 180000); // TODO add to results file
  float eventLoopTime;
  float byteReadRate;

  // compression -> storage format -> readspeed data
  std::map<int, std::map<std::string, ReadSpeedData>> readSpeedData;

  auto formats = withUring ? std::vector<std::string>{"ttree", "rntuple", "rntuple_uring"}
                           : std::vector<std::string>{"ttree", "rntuple"};

  for (const int compression : {/*0,*/ 505, 201, 207}) {
    for (const std::string format : formats) {
      std::string resultsFilePath = std::string(resultsPathBase) + "/" + format + "/readspeed_" +
                                    std::string(physFileType) + "_" + std::to_string(compression) +
                                    ".data";

      std::cout << "** Reading from " << resultsFilePath << std::endl;

      std::ifstream resultsFile(resultsFilePath);

      std::vector<float> eventLoopTimes;
      std::vector<float> byteReadRates;

      while (resultsFile >> eventLoopTime >> byteReadRate) {
        eventLoopTimes.push_back(eventLoopTime);
        byteReadRates.push_back(byteReadRate);
      }

      std::cout << "* Event throughput" << std::endl;

      float eventLoopTimeMedian = TMath::Median(eventLoopTimes.size(), eventLoopTimes.data());

      RVec<float> eventLoopTimeVec(eventLoopTimes.begin(), eventLoopTimes.end());
      auto filteredEventLoopTimes = Filter(eventLoopTimeVec, [eventLoopTimeMedian](float loopTime) {
        return loopTime <= eventLoopTimeMedian + (eventLoopTimeMedian * 0.5) &&
               loopTime >= eventLoopTimeMedian - (eventLoopTimeMedian * 0.5);
      });

      std::cout << "\tRemoved " << eventLoopTimeVec.size() - filteredEventLoopTimes.size()
                << " (of " << eventLoopTimeVec.size() << " total measurements) throughput outliers"
                << std::endl;

      float eventLoopTimeMean = Mean(filteredEventLoopTimes);
      float eventLoopTimeErr = StdErr(filteredEventLoopTimes);
      float eventThroughputMean = nEvents / eventLoopTimeMean;
      float eventThroughputMax = nEvents / (eventLoopTimeMean - eventLoopTimeErr);
      float eventThroughputMin = nEvents / (eventLoopTimeMean + eventLoopTimeErr);
      float eventThroughputErr = (eventThroughputMax - eventThroughputMin) / 2.;

      std::cout << "\tMean = " << eventThroughputMean << "\tError = " << eventThroughputErr
                << std::endl;

      std::cout << "* Byte throughput" << std::endl;

      float byteReadRateMedian = TMath::Median(byteReadRates.size(), byteReadRates.data());

      RVec<float> byteReadRateVec(byteReadRates.begin(), byteReadRates.end());
      auto filteredByteReadRates = Filter(byteReadRateVec, [byteReadRateMedian](float readRate) {
        return readRate <= byteReadRateMedian + (byteReadRateMedian * 0.5) &&
               readRate >= byteReadRateMedian - (byteReadRateMedian * 0.5);
      });

      std::cout << "\tRemoved " << byteReadRateVec.size() - filteredByteReadRates.size() << " (of "
                << byteReadRateVec.size() << " total measurements) throughput outliers"
                << std::endl;

      float byteThroughputMean = Mean(filteredByteReadRates);
      float byteThroughputErr = StdErr(filteredByteReadRates);

      std::cout << "\tMean = " << byteThroughputMean << "\tError = " << byteThroughputErr
                << std::endl;

      readSpeedData[compression][format] =
          ReadSpeedData{new TGraphErrors(), eventThroughputMean, eventThroughputErr,
                        new TGraphErrors(), byteThroughputMean,  byteThroughputErr};
    }

    readSpeedData[compression]["filler"] =
        ReadSpeedData{new TGraphErrors(), 0., 0., new TGraphErrors(), 0., 0.};
  }

  float maxEventThroughput = 0.;
  float maxByteThroughput = 0.;
  for (const auto &[compression, formats] : readSpeedData) {
    for (const auto &[format, data] : formats) {
      int x = getXVal(format, compression, withUring);

      auto gEvent = data.eventThroughputGraph;
      gEvent->SetPoint(0, x + 0, data.eventThroughputMean);
      gEvent->SetPoint(1, x + 2, -1);
      gEvent->SetPointError(0, 0, data.eventThroughputErr);
      maxEventThroughput =
          std::max(maxEventThroughput, data.eventThroughputMean + data.eventThroughputErr);

      auto gByte = data.byteThroughputGraph;
      gByte->SetPoint(0, x + 0, data.byteThroughputMean);
      gByte->SetPoint(1, x + 2, -1);
      gByte->SetPointError(0, 0, data.byteThroughputErr);
      maxByteThroughput =
          std::max(maxByteThroughput, data.byteThroughputMean + data.byteThroughputErr);
    }
  }

  drawGraph(readSpeedData, maxEventThroughput * 1.1, physFileType, medium, withUring, "event",
            save);
  drawGraph(readSpeedData, maxByteThroughput * 1.25, physFileType, medium, withUring, "byte", save);
}

void plot_overview_per_medium() {
  SetStyle();

  plot("results/chep/ssd", "SSD", "data", true, true);
  plot("results/chep/hdd", "HDD", "data", true, true);
  plot("results/chep/xrootd", "XRootD", "data", false, true);
  plot("results/chep/tmpfs", "tmpfs", "data", false, true);
}
