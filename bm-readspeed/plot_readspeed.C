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
#include <TPad.h>

using namespace ROOT::VecOps;

const bool kRemoveOutliers = false;

struct ReadSpeedData {
  TGraphErrors *eventThroughputGraph;
  float eventThroughputMean, eventThroughputErr;

  TGraphErrors *byteThroughputGraph;
  float byteThroughputMean, byteThroughputErr;
};

void plotOverview(std::string_view resultsPathBase, std::string_view medium,
                  std::string_view physFileType = "data", bool withUring = true,
                  bool save = false) {
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

  for (const int compression : {0, 505, 201, 207}) {
    for (const std::string format : formats) {
      std::string resultsFilePath = std::string(resultsPathBase) + "/" + format + "/readspeed_" +
                                    std::string(physFileType) + "_" + std::to_string(compression) +
                                    ".data";

      std::ifstream resultsFile(resultsFilePath);

      std::vector<float> eventLoopTimes;
      std::vector<float> byteReadRates;

      while (resultsFile >> eventLoopTime >> byteReadRate) {
        eventLoopTimes.push_back(eventLoopTime);
        byteReadRates.push_back(byteReadRate);
      }

      RVec<float> eventLoopTimeVec(eventLoopTimes.begin(), eventLoopTimes.end());
      float eventLoopTimeMean = Mean(eventLoopTimeVec);
      float eventLoopTimeErr = StdErr(eventLoopTimeVec);
      float eventThroughputMean = nEvents / eventLoopTimeMean;
      float eventThroughputMax = nEvents / (eventLoopTimeMean - eventLoopTimeErr);
      float eventThroughputMin = nEvents / (eventLoopTimeMean + eventLoopTimeErr);
      float eventThroughputErr = (eventThroughputMax - eventThroughputMin) / 2.;

      if (kRemoveOutliers) {
        float eventThroughputStdDev = StdDev(eventLoopTimeVec);
        auto filteredEvents =
            Filter(eventLoopTimeVec, [eventThroughputMean, eventThroughputStdDev](float x) {
              float z = (x - eventThroughputMean) / eventThroughputStdDev;
              return z < -3. || z > 3.;
            });

        // std::cout << filteredEvents.size() << std::endl;
      }

      RVec<float> byteReadRateVec(byteReadRates.begin(), byteReadRates.end());
      float byteThroughputMean = Mean(byteReadRateVec);
      float byteThroughputErr = StdErr(byteReadRateVec);

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

  // Title pad
  auto padEventTitle = new TPad("padETitle", "padEventTitle", 0.0, 0.93, 1.0, 0.97);
  padEventTitle->SetTopMargin(0.02);
  padEventTitle->SetBottomMargin(0.01);
  padEventTitle->SetLeftMargin(0.1);
  padEventTitle->SetRightMargin(0.005);
  padEventTitle->SetFillStyle(4000);
  padEventTitle->SetFrameFillStyle(4000);
  padEventTitle->Draw();
  canvasEventThroughput->cd();

  // TTree vs RNTuple read throughput speed
  auto padEventThroughput =
      new TPad("padEventThroughput", "padEventThroughput", 0.0, 0.03, 1.0, 0.92);
  padEventThroughput->SetTopMargin(0.055);
  padEventThroughput->SetBottomMargin(0.08);
  padEventThroughput->SetLeftMargin(0.085);
  padEventThroughput->SetRightMargin(0.04);
  padEventThroughput->SetFillStyle(4000);
  padEventThroughput->SetFrameFillStyle(4000);
  padEventThroughput->Draw();
  canvasEventThroughput->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE TITLE                                                           //
  //--------------------------------------------------------------------------//

  padEventTitle->cd();
  // auto padEventCenter = padEventTitle->GetBBoxCenter();
  auto eventTitle = new TText(
      0.5, 0.5, Form("TTree vs. RNTuple event throughput (%s)", std::string(medium).c_str()));
  eventTitle->SetBBoxCenter(padEventTitle->GetBBoxCenter());
  eventTitle->SetTextColor(kBlack);
  eventTitle->SetTextSize(1.00);
  eventTitle->SetTextAlign(23);
  eventTitle->Draw();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//
  int nBins = withUring ? 16 : 12;
  int binStart = nBins / 4 + 1;
  int binInterval = nBins / 2;

  maxEventThroughput *= 1.1;

  TH1F *helperEventThroughput = new TH1F("", "", nBins, 0, nBins);
  helperEventThroughput->GetXaxis()->SetTickSize(0);
  helperEventThroughput->GetXaxis()->SetNdivisions(nBins * 2);
  helperEventThroughput->GetXaxis()->SetLabelOffset(0.01);
  helperEventThroughput->GetYaxis()->SetTickSize(0.01);
  helperEventThroughput->GetYaxis()->SetLabelSize(0.0375);
  helperEventThroughput->GetYaxis()->SetTitle("Events / s");
  helperEventThroughput->GetYaxis()->SetTitleSize(0.045);
  helperEventThroughput->GetYaxis()->SetTitleOffset(0.85);
  helperEventThroughput->SetMinimum(0);
  helperEventThroughput->SetMaximum(maxEventThroughput);

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "no compression");
    } else if (i == binStart + binInterval) {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "zstd");
    } else if (i == binStart + (2 * binInterval)) {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 1)");
    } else if (i == binStart + (3 * binInterval)) {
      helperEventThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 7)");
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
          // val << " #pm " << std::fixed << readSpeedData[compression][format].eventThroughputMean;

          TLatex tval;
          tval.SetTextSize(0.02);
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
    leg->AddEntry(readSpeedData[505]["rntuple_uring"].eventThroughputGraph, "RNTuple (w/ liburing)",
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

  // Title pad
  auto padByteTitle = new TPad("padByteTitle", "padByteTitle", 0.0, 0.93, 1.0, 0.97);
  padByteTitle->SetTopMargin(0.02);
  padByteTitle->SetBottomMargin(0.01);
  padByteTitle->SetLeftMargin(0.1);
  padByteTitle->SetRightMargin(0.005);
  padByteTitle->SetFillStyle(4000);
  padByteTitle->SetFrameFillStyle(4000);
  padByteTitle->Draw();
  canvasByteThroughput->cd();

  // TTree vs RNTuple read throughput speed
  auto padByteThroughput = new TPad("padByteThroughput", "padByteThroughput", 0.0, 0.03, 1.0, 0.92);
  padByteThroughput->SetTopMargin(0.055);
  padByteThroughput->SetBottomMargin(0.08);
  padByteThroughput->SetLeftMargin(0.08);
  padByteThroughput->SetRightMargin(0.04);
  padByteThroughput->SetFillStyle(4000);
  padByteThroughput->SetFrameFillStyle(4000);
  padByteThroughput->Draw();
  canvasByteThroughput->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE TITLE                                                           //
  //--------------------------------------------------------------------------//

  padByteTitle->cd();
  // auto padByteCenter = padByteTitle->GetBBoxCenter();
  auto byteTitle = new TText(
      0.5, 0.5,
      Form("TTree vs. RNTuple raw compressed byte throughput (%s)", std::string(medium).c_str()));
  byteTitle->SetBBoxCenter(padByteTitle->GetBBoxCenter());
  byteTitle->SetTextColor(kBlack);
  byteTitle->SetTextSize(1.00);
  byteTitle->SetTextAlign(23);
  byteTitle->Draw();

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
  helperByteThroughput->GetYaxis()->SetLabelSize(0.0375);
  helperByteThroughput->GetYaxis()->SetTitle("MB / s");
  helperByteThroughput->GetYaxis()->SetTitleSize(0.045);
  helperByteThroughput->GetYaxis()->SetTitleOffset(0.8);
  helperByteThroughput->SetMinimum(0);
  helperByteThroughput->SetMaximum(maxByteThroughput);

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "no compression");
    } else if (i == binStart + binInterval) {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "zstd");
    } else if (i == binStart + (2 * binInterval)) {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 1)");
    } else if (i == binStart + (3 * binInterval)) {
      helperByteThroughput->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 7)");
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
          tval.SetTextSize(0.02);
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

void plot_readspeed() {
  SetStyle();

  plotOverview("results/local_rdf3", "SSD (local)", "data", false, false);
  // plotOverview("results/ssd_rdf", "SSD", "data", true, true);
  // plotOverview("results/rdf_cached", "cached", "mc", false);
  // plotOverview("results/cached_rdf", "cached", "mc", false, false);
  // plotOverview("results/xrootd_rdf", "XRootD", "mc", false, false);
}
