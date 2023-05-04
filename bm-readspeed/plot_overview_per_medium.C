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

  //==========================================================================//
  // EVENT THROUGHPUT                                                         //
  //==========================================================================//

  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//

  TCanvas *canvasEventThroughput =
      new TCanvas(Form("canvas_event_throughput_%s", std::string(medium).c_str()),
                  Form("canvas_event_throughput_%s", std::string(medium).c_str()), 1200, 600);
  canvasEventThroughput->cd();

  // TTree vs RNTuple read throughput speed
  auto padEventThroughput =
      new TPad("padEventThroughput", "padEventThroughput", 0.0, 0.0, 1.0, 1.0);
  padEventThroughput->SetTopMargin(0.055);
  padEventThroughput->SetBottomMargin(0.08);
  padEventThroughput->SetLeftMargin(0.08);
  padEventThroughput->SetRightMargin(0.04);
  padEventThroughput->SetFillStyle(4000);
  padEventThroughput->SetFrameFillStyle(4000);
  padEventThroughput->Draw();
  canvasEventThroughput->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//
  int nBins = withUring ? 12 : 9;
  int binStart = nBins / 3 + 1;
  int binInterval = nBins / 2 + 2;

  maxEventThroughput *= 1.1;

  TH1F *helperEventThroughput = new TH1F("", "", nBins, 0, nBins);
  helperEventThroughput->GetXaxis()->SetTickSize(0);
  helperEventThroughput->GetXaxis()->SetNdivisions(nBins * 2);
  helperEventThroughput->GetXaxis()->SetLabelOffset(0.01);
  helperEventThroughput->GetYaxis()->SetTickSize(0.01);
  helperEventThroughput->GetYaxis()->SetLabelSize(0.035);
  helperEventThroughput->GetYaxis()->SetTitle("Events / s");
  helperEventThroughput->GetYaxis()->SetTitleSize(0.035);
  helperEventThroughput->GetYaxis()->SetTitleOffset(1.1);
  helperEventThroughput->SetMinimum(0);
  helperEventThroughput->SetMaximum(maxEventThroughput);
  helperEventThroughput->SetTitleSize(0.06);
  helperEventThroughput->SetTitle(
      Form("DAOD_PHYS event read throughput, %s", std::string(medium).c_str()));

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "zstd");
    } else if (i == binStart + binInterval) {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (level 1)");
    } else if (i == binStart + (2 * binInterval)) {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (level 7)");
    } else {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  padEventThroughput->cd();
  gPad->SetGridy();

  helperEventThroughput->Draw();

  for (const auto &[compression, formats] : readSpeedData) {
    for (const auto &[format, data] : formats) {
      data.eventThroughputGraph->SetLineColor(12);
      data.eventThroughputGraph->SetMarkerColor(12);
      data.eventThroughputGraph->SetFillColor(colors.at(format));
      data.eventThroughputGraph->SetFillStyle(styles.at(format));
      data.eventThroughputGraph->SetLineWidth(2);
      data.eventThroughputGraph->Draw("B1");
      data.eventThroughputGraph->Draw("P");

      if (format == "rntuple" || format == "rntuple_uring") {
        for (int i = 0; i < data.eventThroughputGraph->GetN(); ++i) {

          double x, y;
          data.eventThroughputGraph->GetPoint(i, x, y);

          if (y < 0)
            continue;

          std::ostringstream val;
          val.precision(1);
          val << "#times" << std::fixed
              << y / readSpeedData[compression]["ttree"].eventThroughputMean;

          TLatex tval;
          tval.SetTextColor(kWhite);
          tval.SetTextSize(0.03);
          tval.SetTextAlign(21);
          tval.DrawLatex(x, maxEventThroughput * 0.025, val.str().c_str());
        }
      }
    }
  }

  for (unsigned i = 0; i < nBins; i += (binInterval / 2)) {
    TLine *line = new TLine(i, 0, i, maxEventThroughput);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *leg = new TLegend(0.825, 0.8, 0.955, 0.935);
  leg->AddEntry(readSpeedData[505]["ttree"].eventThroughputGraph, "TTree", "F");
  leg->AddEntry(readSpeedData[505]["rntuple"].eventThroughputGraph, "RNTuple", "F");
  if (withUring) {
    leg->AddEntry(readSpeedData[505]["rntuple_uring"].eventThroughputGraph, "RNTuple (w/ io_uring)",
                  "F");
  }
  leg->SetNColumns(1);
  leg->SetMargin(0.15);
  if (!withUring)
    leg->SetTextSize(0.035);
  leg->Draw();

  TText l;
  l.SetTextSize(0.025);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.918, 0.785, "95% CL");

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvasEventThroughput->Print(Form("figures/readspeed_event_throughput_%s_%s.pdf",
                                      std::string(medium).c_str(),
                                      std::string(physFileType).c_str()));
    canvasEventThroughput->Print(Form("figures/readspeed_event_throughput_%s_%s.png",
                                      std::string(medium).c_str(),
                                      std::string(physFileType).c_str()));
  }

  //==========================================================================//
  // RAW BYTE THROUGHPUT                                                      //
  //==========================================================================//

  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//

  TCanvas *canvasByteThroughput =
      new TCanvas(Form("canvas_byte_throughput_%s", std::string(medium).c_str()),
                  Form("canvas_byte_throughput_%s", std::string(medium).c_str()), 1200, 600);
  canvasByteThroughput->cd();

  // TTree vs RNTuple read throughput speed
  auto padByteThroughput = new TPad("padByteThroughput", "padByteThroughput", 0.0, 0.0, 1.0, 1.0);
  padByteThroughput->SetTopMargin(0.055);
  padByteThroughput->SetBottomMargin(0.08);
  padByteThroughput->SetLeftMargin(0.08);
  padByteThroughput->SetRightMargin(0.04);
  padByteThroughput->SetFillStyle(4000);
  padByteThroughput->SetFrameFillStyle(4000);
  padByteThroughput->Draw();
  canvasByteThroughput->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//
  maxByteThroughput *= 1.25;

  TH1F *helperByteThroughput = new TH1F("", "", nBins, 0, nBins);
  helperByteThroughput->GetXaxis()->SetTickSize(0);
  helperByteThroughput->GetXaxis()->SetNdivisions(nBins * 2);
  helperByteThroughput->GetXaxis()->SetLabelOffset(0.01);
  // helperByteThroughput->GetYaxis()->SetMaxDigits(4);
  helperByteThroughput->GetYaxis()->SetTickSize(0.01);
  helperByteThroughput->GetYaxis()->SetLabelSize(0.035);
  helperByteThroughput->GetYaxis()->SetTitle("MB / s");
  helperByteThroughput->GetYaxis()->SetTitleSize(0.035);
  helperByteThroughput->GetYaxis()->SetTitleOffset(1.1);
  helperByteThroughput->SetMinimum(0);
  helperByteThroughput->SetMaximum(maxByteThroughput);
  helperByteThroughput->SetTitle(
      Form("DAOD_PHYS raw byte read throughput, %s", std::string(medium).c_str()));

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "zstd");
    } else if (i == binStart + binInterval) {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (level 1)");
    } else if (i == binStart + (2 * binInterval)) {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (level 7)");
    } else {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  padByteThroughput->cd();
  gPad->SetGridy();

  helperByteThroughput->Draw();

  for (const auto &[compression, formats] : readSpeedData) {
    for (const auto &[format, data] : formats) {
      data.byteThroughputGraph->SetLineColor(12);
      data.byteThroughputGraph->SetMarkerColor(12);
      data.byteThroughputGraph->SetFillColor(colors.at(format));
      data.byteThroughputGraph->SetFillStyle(styles.at(format));
      data.byteThroughputGraph->SetLineWidth(2);
      data.byteThroughputGraph->Draw("B1");
      data.byteThroughputGraph->Draw("P");

      if (format == "rntuple" || format == "rntuple_uring") {
        for (int i = 0; i < data.byteThroughputGraph->GetN(); ++i) {

          double x, y;
          data.byteThroughputGraph->GetPoint(i, x, y);

          if (y < 0)
            continue;

          std::ostringstream val;
          val.precision(1);
          val << "#times" << std::fixed
              << y / readSpeedData[compression]["ttree"].byteThroughputMean;
          // val << " #pm " << std::fixed << readSpeedData[compression][format].byteThroughputMean;

          TLatex tval;
          tval.SetTextColor(kWhite);
          tval.SetTextSize(0.03);
          tval.SetTextAlign(21);
          tval.DrawLatex(x, maxByteThroughput * 0.025, val.str().c_str());
        }
      }
    }
  }

  for (unsigned i = 0; i < nBins; i += (binInterval / 2)) {
    TLine *line = new TLine(i, 0, i, maxByteThroughput);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  leg->Draw();
  l.DrawTextNDC(0.918, 0.785, "95% CL");

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvasByteThroughput->Print(Form("figures/readspeed_byte_throughput_%s_%s.pdf",
                                     std::string(medium).c_str(),
                                     std::string(physFileType).c_str()));
    canvasByteThroughput->Print(Form("figures/readspeed_byte_throughput_%s_%s.png",
                                     std::string(medium).c_str(),
                                     std::string(physFileType).c_str()));
  }
}

void plot_overview_per_medium() {
  SetStyle();

  // plot("results/chep/ssd", "SSD", "data", true, true);
  // plot("results/chep/hdd", "HDD", "data", true, true);
  // plot("results/chep/cached", "cached", "data", false, true);
  plot("results/chep/xrootd", "XRootD", "data", false, true);
  plot("results/chep/tmpfs", "tmpfs", "data", false, false);
}
