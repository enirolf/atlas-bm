#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <TPad.h>

#include <ROOT/RVec.hxx>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../bm-utils/plot_util.C"

using namespace ROOT::VecOps;

const bool kRemoveOutliers = false;

struct ReadSpeedData {
  TGraphErrors *throughputGraph;
  float throughputMean, throughputErr;
};

void drawEventThroughput(
    std::map<int, std::map<std::string, ReadSpeedData>> &readSpeedData,
    float maxThroughput, std::string_view physFileType, std::string_view medium,
    bool withUring, bool save = false) {
  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//
  TCanvas *canvas = new TCanvas(
      Form("canvas_event_throughput_%s", std::string(medium).c_str()),
      Form("canvas_event_throughput_%s", std::string(medium).c_str()), 1200, 800);
  canvas->SetFillStyle(4000);
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto pad = new TPad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad->SetTopMargin(0.1);
  pad->SetBottomMargin(0.18);
  pad->SetLeftMargin(0.1);
  pad->SetRightMargin(0.02);
  pad->SetFillStyle(4000);
  pad->SetFrameFillStyle(4000);
  pad->Draw();
  canvas->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE GRAPH                                                           //
  //--------------------------------------------------------------------------//
  int nBins = withUring ? 16 : 12;
  int binStart = withUring ? 3 : 2;
  int binInterval = withUring ? 4 : 3;

  float binOffset = withUring ? 0 : 0.5;

  TH1F *helper = new TH1F("", "", nBins, binOffset, nBins + binOffset);
  helper->GetXaxis()->SetTitle("Compression method");
  helper->GetXaxis()->SetTitleSize(0.05);
  helper->GetXaxis()->SetTitleOffset(1.45);
  helper->GetXaxis()->SetTickSize(0);
  helper->GetXaxis()->SetNdivisions(nBins);
  helper->GetXaxis()->SetLabelOffset(0.025);
  helper->GetYaxis()->SetMaxDigits(3);
  helper->GetYaxis()->SetTickSize(0.01);
  helper->GetYaxis()->SetLabelSize(0.05);
  helper->GetYaxis()->SetTitle("Events / s");
  helper->GetYaxis()->SetTitleSize(0.05);
  helper->GetYaxis()->SetTitleOffset(1.);
  helper->SetMinimum(0);
  helper->SetMaximum(maxThroughput);

  float labelSize = 0.05;

  for (int i = 0; i <= nBins + 1; i++) {
    if (i == binStart) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "lz4");
    } else if (i == binStart + binInterval) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "zstd*");
    } else if (i == binStart + (2 * binInterval)) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1,
                                      "lzma (lvl 1)");
    } else if (i == binStart + (3 * binInterval)) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1,
                                      "lzma (lvl 7)");
    } else {
      helper->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  pad->cd();
  gPad->SetGridy();

  helper->Draw();

  for (const auto &[compression, formats] : readSpeedData) {
    for (const auto &[format, data] : formats) {
      auto graph = data.throughputGraph;

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

          if (y < 0) continue;

          y /= readSpeedData[compression]["ttree"].throughputMean;

          std::ostringstream val;
          val.precision(1);
          val << "#times" << std::fixed << y;

          TLatex tval;
          if (withUring)
            tval.SetTextSize(0.04);
          else
            tval.SetTextSize(0.045);
          tval.SetTextAlign(21);

          if (y < 0.7) {
            tval.SetTextColor(kBlack);
            tval.DrawLatex(x, maxThroughput * 0.05, val.str().c_str());
          } else {
            tval.SetTextColor(kWhite);
            tval.DrawLatex(x, maxThroughput * 0.025, val.str().c_str());
          }
        }
      }
    }
  }

  for (unsigned i = 0; i < nBins; i += (binInterval)) {
    TLine *line = new TLine(i + binOffset, 0, i + binOffset, maxThroughput);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *leg = new TLegend(0.8, 0.75, 0.97, 0.88);
  leg->AddEntry(readSpeedData[505]["ttree"].throughputGraph, "TTree", "F");
  leg->AddEntry(readSpeedData[505]["rntuple"].throughputGraph, "RNTuple", "F");
  if (withUring) {
    leg->AddEntry(readSpeedData[505]["rntuple_uring"].throughputGraph,
                  "RNTuple (w/ io_uring)", "F");
  }
  leg->SetNColumns(1);
  leg->Draw();

  TText clAnnot;
  clAnnot.SetTextSize(0.03);
  clAnnot.SetTextAlign(33);
  clAnnot.SetTextFont(42);
  clAnnot.DrawTextNDC(0.98, 0.94, "95% CL");

  TText zstdAnnot;
  zstdAnnot.SetTextSize(0.035);
  zstdAnnot.SetTextAlign(33);
  zstdAnnot.SetTextFont(42);
  zstdAnnot.DrawTextNDC(0.98, 0.035, "*current ATLAS default");

  TLatex title;
  title.SetTextSize(0.045);
  title.SetTextAlign(23);
  title.SetTextFont(42);
  if (physFileType == "mc")
    title.DrawLatexNDC(0.55, 0.99, Form("%s event throughput, MC", std::string(medium).c_str()));
  else
    title.DrawLatexNDC(0.55, 0.99, Form("%s event throughput, data", std::string(medium).c_str()));

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//
  if (save) {
    canvas->Print(Form("figures/chep_proc/readspeed_event_throughput_%s_%s.pdf",
                       std::string(medium).c_str(),
                       std::string(physFileType).c_str()));
  }
}

void drawByteThroughput(std::map<std::string, ReadSpeedData> &readSpeedData,
                        float maxThroughput, std::string_view physFileType,
                        std::string_view medium, bool withUring,
                        bool save = false) {
  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//
  TCanvas *canvas = new TCanvas(
      Form("canvas_byte_throughput_%s", std::string(medium).c_str()),
      Form("canvas_byte_throughput_%s", std::string(medium).c_str()), 600, 800);
  canvas->SetFillStyle(4000);
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto pad = new TPad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad->SetTopMargin(0.1);
  pad->SetBottomMargin(0.18);
  pad->SetLeftMargin(0.15);
  if (maxThroughput < 1000) {
    pad->SetLeftMargin(0.18);
  }
  pad->SetRightMargin(0.1);
  pad->SetFillStyle(4000);
  pad->SetFrameFillStyle(4000);
  pad->Draw();
  canvas->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE GRAPH                                                           //
  //--------------------------------------------------------------------------//
  int nBins = withUring ? 4 : 3;
  int binStart = withUring ? 3 : 2;
  int binInterval = withUring ? 4 : 3;

  TH1F *helper = new TH1F("", "", nBins, 0, nBins);
  helper->GetXaxis()->SetTickSize(0);
  helper->GetXaxis()->SetNdivisions(nBins);
  helper->GetXaxis()->SetLabelOffset(0.025);
  helper->GetYaxis()->SetMaxDigits(3);
  helper->GetYaxis()->SetTickSize(0.01);
  helper->GetYaxis()->SetLabelSize(0.06);
  helper->GetYaxis()->SetLabelOffset(0.01);
  helper->GetYaxis()->SetTitle("MB / s");
  helper->GetYaxis()->SetTitleSize(0.06);
  helper->GetYaxis()->SetTitleOffset(1.2);
   if (maxThroughput < 1000) {
    helper->GetYaxis()->SetTitleOffset(1.5);
  }
  helper->SetMinimum(0);
  helper->SetMaximum(maxThroughput);

  float labelSize = 0.03;

  for (int i = 0; i <= nBins + 1; i++) {
    helper->GetXaxis()->ChangeLabel(i, -1, 0);
  }

  pad->cd();
  gPad->SetGridy();

  helper->Draw();

  for (const auto &[format, data] : readSpeedData) {
    auto graph = data.throughputGraph;

    graph->SetLineColor(12);
    graph->SetMarkerColor(12);
    graph->SetFillColor(colors.at(format));
    graph->SetFillStyle(styles.at(format));
    graph->SetLineWidth(2);
    graph->Draw("B1");
    graph->Draw("P");

    for (int i = 0; i < graph->GetN(); ++i) {
      double x, y;
      graph->GetPoint(i, x, y);

      if (y < 0) continue;
      if (format == "rntuple" || format == "rntuple_uring") {
        y /= readSpeedData["ttree"].throughputMean;

        std::ostringstream val;
        val.precision(1);
        val << "#times" << std::fixed << y;

        TLatex tval;
        tval.SetTextColor(kWhite);
        if (withUring)
          tval.SetTextSize(0.06);
        else
          tval.SetTextSize(0.07);
        tval.SetTextAlign(21);
        tval.DrawLatex(x, maxThroughput * 0.025, val.str().c_str());
      }

      TLatex tlabel;
      tlabel.SetTextFont(42);
      if (withUring)
        tlabel.SetTextSize(0.05);
      else
        tlabel.SetTextSize(0.06);
      tlabel.SetTextAlign(23);
      if (format == "ttree")
        tlabel.DrawLatex(x, maxThroughput * -0.03, "TTree");
      else if (format == "rntuple")
        tlabel.DrawLatex(x, maxThroughput * -0.03, "RNTuple");
      else
        tlabel.DrawLatex(x, maxThroughput * -0.0605, "#splitline{RNTuple}{#scale[0.7]{(w/ io_uring)}}");
    }
  }

  TText l;
  l.SetTextSize(0.045);
  l.SetTextAlign(13);
  l.SetTextFont(42);
  l.DrawTextNDC(0.75, 0.94, "95% CL");

  TLatex title;
  title.SetTextSize(0.06);
  title.SetTextAlign(23);
  title.SetTextFont(42);
  if (physFileType == "mc")
    title.DrawLatexNDC(0.525, 0.99, Form("%s raw I/O throughput, MC", std::string(medium).c_str()));
  else
    title.DrawLatexNDC(0.525, 0.99, Form("%s raw I/O throughput, data", std::string(medium).c_str()));

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//
  if (save) {
    canvas->Print(Form("figures/readspeed_byte_throughput_%s_%s.pdf",
                       std::string(medium).c_str(),
                       std::string(physFileType).c_str()));
  }
}

void plot(std::string_view resultsPathBase, std::string_view medium,
          std::string_view physFileType = "data", bool withUring = true,
          bool save = false) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//
  float nEvents =
      (physFileType == "data" ? 209062 : 180000);  // TODO add to results file
  float eventLoopTime;
  float byteReadRate;

  // compression -> storage format -> readspeed data
  std::map<int, std::map<std::string, ReadSpeedData>> eventThroughputData;
  std::map<std::string, ReadSpeedData> byteThroughputData;

  auto formats =
      withUring ? std::vector<std::string>{"ttree", "rntuple", "rntuple_uring"}
                : std::vector<std::string>{"ttree", "rntuple"};

  for (const std::string format : formats) {
    std::vector<float> byteReadRates;

    for (const int compression : {404, 505, 201, 207}) {
      std::string resultsFilePath = std::string(resultsPathBase) + "/" +
                                    format + "/readspeed_" +
                                    std::string(physFileType) + "_" +
                                    std::to_string(compression) + ".data";

      std::cout << "** Reading from " << resultsFilePath << std::endl;

      std::ifstream resultsFile(resultsFilePath);

      std::vector<float> eventLoopTimes;

      while (resultsFile >> eventLoopTime >> byteReadRate) {
        eventLoopTimes.push_back(eventLoopTime);
        byteReadRates.push_back(byteReadRate);
      }

      std::cout << "* Event throughput" << std::endl;

      float eventLoopTimeMedian =
          TMath::Median(eventLoopTimes.size(), eventLoopTimes.data());

      RVec<float> eventLoopTimeVec(eventLoopTimes.begin(),
                                   eventLoopTimes.end());
      auto filteredEventLoopTimes =
          Filter(eventLoopTimeVec, [eventLoopTimeMedian](float loopTime) {
            return loopTime <=
                       eventLoopTimeMedian + (eventLoopTimeMedian * 0.5) &&
                   loopTime >=
                       eventLoopTimeMedian - (eventLoopTimeMedian * 0.5);
          });

      std::cout << "\tRemoved "
                << eventLoopTimeVec.size() - filteredEventLoopTimes.size()
                << " (of " << eventLoopTimeVec.size()
                << " total measurements) throughput outliers" << std::endl;

      float eventLoopTimeMean = Mean(filteredEventLoopTimes);
      float eventLoopTimeErr = StdErr(filteredEventLoopTimes);
      float eventThroughputMean = nEvents / eventLoopTimeMean;
      float eventThroughputMax =
          nEvents / (eventLoopTimeMean - eventLoopTimeErr);
      float eventThroughputMin =
          nEvents / (eventLoopTimeMean + eventLoopTimeErr);
      float eventThroughputErr = (eventThroughputMax - eventThroughputMin) / 2.;

      std::cout << "\tMean = " << eventThroughputMean
                << "\tError = " << eventThroughputErr << std::endl;

      eventThroughputData[compression][format] = ReadSpeedData{
          new TGraphErrors(), eventThroughputMean, eventThroughputErr};
      eventThroughputData[compression]["filler"] =
          ReadSpeedData{new TGraphErrors(), 0., 0.};
    }

    std::cout << "* Byte throughput" << std::endl;

    float byteReadRateMedian =
        TMath::Median(byteReadRates.size(), byteReadRates.data());

    RVec<float> byteReadRateVec(byteReadRates.begin(), byteReadRates.end());
    auto filteredByteReadRates =
        Filter(byteReadRateVec, [byteReadRateMedian](float readRate) {
          return readRate <= byteReadRateMedian + (byteReadRateMedian * 0.5) &&
                 readRate >= byteReadRateMedian - (byteReadRateMedian * 0.5);
        });

    std::cout << "\tRemoved "
              << byteReadRateVec.size() - filteredByteReadRates.size()
              << " (of " << byteReadRateVec.size()
              << " total measurements) throughput outliers" << std::endl;

    float byteThroughputMean = Mean(filteredByteReadRates);
    float byteThroughputErr = StdErr(filteredByteReadRates);

    std::cout << "\tMean = " << byteThroughputMean
              << "\tError = " << byteThroughputErr << std::endl;

    byteThroughputData[format] = ReadSpeedData{
        new TGraphErrors(), byteThroughputMean, byteThroughputErr};
  }

  float maxEventThroughput = 0.;
  for (const auto &[compression, formats] : eventThroughputData) {
    for (const auto &[format, data] : formats) {
      float x = getXVal(format, compression, withUring);
      if (!withUring)
        x += 0.5;

      auto gEvent = data.throughputGraph;
      gEvent->SetPoint(0, x + 0, data.throughputMean);
      gEvent->SetPoint(1, x + 2, -1);
      gEvent->SetPointError(0, 0, data.throughputErr);
      maxEventThroughput = std::max(maxEventThroughput,
                                    data.throughputMean + data.throughputErr);
    }
  }

  float maxByteThroughput = 0.;
  for (const auto &[format, data] : byteThroughputData) {
    float x = getXVal(format, 404, withUring);

    auto gByte = data.throughputGraph;
    gByte->SetPoint(0, x + 0, data.throughputMean);
    gByte->SetPoint(1, x + 2, -1);
    gByte->SetPointError(0, 0, data.throughputErr);
    maxByteThroughput =
        std::max(maxByteThroughput, data.throughputMean + data.throughputErr);
  }

  drawEventThroughput(eventThroughputData, maxEventThroughput * 1.1,
                      physFileType, medium, withUring, save);
  drawByteThroughput(byteThroughputData, maxByteThroughput * 1.05, physFileType,
                     medium, withUring, save);
}

void plot_overview_per_medium() {
  SetStyle();

  plot("results/ssd", "SSD", "mc", true, true);
  plot("results/hdd", "HDD", "mc", true, true);
  plot("results/xrootd", "XRootD", "mc", false, true);
  plot("results/tmpfs", "tmpfs", "mc", false, true);

  plot("results/ssd", "SSD", "data", true, true);
  plot("results/hdd", "HDD", "data", true, true);
  plot("results/xrootd", "XRootD", "data", false, true);
  plot("results/tmpfs", "tmpfs", "data", false, true);
}
