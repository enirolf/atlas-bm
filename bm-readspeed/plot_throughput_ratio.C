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
  float eventSpeedupMean, eventSpeedupErr;
  float byteSpeedupMean, byteSpeedupErr;
};

void drawGraph(std::map<int, std::map<std::string, TGraphErrors *>> &graphs, float maxThrougput,
               std::string_view plotKind, bool save = false) {
  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//

  TCanvas *canvas =
      new TCanvas(Form("canvas_througput_%s", std::string(plotKind).c_str()),
                  Form("canvas_througput_%s", std::string(plotKind).c_str()), 1200, 1000);
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
  pad->SetFillStyle(4000);
  pad->Draw();
  canvas->cd();

  int nBins = 20;
  int binStart = 8;
  int binInterval = 10;

  //--------------------------------------------------------------------------//
  // DRAW THE GRAPH                                                           //
  //--------------------------------------------------------------------------//

  maxThrougput *= 1.1;

  TH1F *helper = new TH1F("", "", nBins, 0, nBins);
  helper->GetXaxis()->SetTickSize(0);
  helper->GetXaxis()->SetNdivisions(nBins * 2);
  helper->GetXaxis()->SetLabelOffset(0.02);
  helper->GetYaxis()->SetTickSize(0.01);
  helper->GetYaxis()->SetLabelSize(0.045);
  helper->GetYaxis()->SetTitle("Throughput ratio (RNTuple / TTree)");
  helper->GetYaxis()->SetTitleSize(0.05);
  helper->GetYaxis()->SetTitleOffset(1.0);
  helper->SetMinimum(0);
  helper->SetMaximum(maxThrougput);

  float labelSize = 0.05;

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "zstd");
    } else if (i == binStart + binInterval) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "lz4");
    } else if (i == binStart + (2 * binInterval)) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "lzma (lvl 1)");
    } else if (i == binStart + (3 * binInterval)) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "lzma (lvl 7)");
    } else {
      helper->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  pad->cd();
  gPad->SetGridy();

  helper->Draw();

  for (const auto &[compression, media] : graphs) {
    for (const auto &[medium, graph] : media) {
      graph->SetLineColor(12);
      graph->SetMarkerColor(12);
      graph->SetFillColor(colors.at(medium));
      graph->SetLineWidth(2);
      graph->Draw("B1");
      graph->Draw("P");

      for (int i = 0; i < graph->GetN(); ++i) {

        double x, y;
        graph->GetPoint(i, x, y);

        if (y <= 0)
          continue;

        std::ostringstream val;
        val.precision(1);
        val << "#times" << std::fixed << y;

        TLatex tval;
        tval.SetTextColor(kWhite);
        tval.SetTextSize(0.03);
        tval.SetTextAlign(21);
        tval.DrawLatex(x, maxThrougput * 0.025, val.str().c_str());
      }
    }
  }

  for (unsigned i = 0; i < nBins; i += (binInterval / 2)) {
    TLine *line = new TLine(i, 0, i, maxThrougput);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *leg = new TLegend(0.65, 0.765, 0.975, 0.935);
  leg->AddEntry(graphs[505]["ssd"], "SSD", "F");
  leg->AddEntry(graphs[505]["hdd"], "HDD", "F");
  leg->AddEntry(graphs[505]["tmpfs"], "RAM", "F");
  leg->AddEntry(graphs[505]["xrootd"], "XRootD (100GbE, 0.3ms)", "F");
  leg->SetNColumns(1);
  // leg->SetMargin(0.15);
  leg->Draw();

  TText l;
  l.SetTextSize(0.025);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.9, 0.75, "95% CL");

  TLine *lineOne = new TLine(0, 1, 15, 1);
  lineOne->SetLineColor(kBlack);
  lineOne->SetLineStyle(7);
  lineOne->SetLineWidth(2);
  lineOne->Draw();

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvas->Print(Form("figures/readspeed_throughput_ratio_%s.pdf", std::string(plotKind).c_str()));
    canvas->Print(Form("figures/readspeed_throughput_ratio_%s.png", std::string(plotKind).c_str()));
  }
}

void plot(std::string_view resultsPathBase, std::string_view physFileType = "data",
          bool save = false) {
  //--------------------------------------------------------------------------//
  // PREPARE THE DATA                                                         //
  //--------------------------------------------------------------------------//
  float nEvents = (physFileType == "data" ? 209062 : 180000); // TODO add to results file
  float eventLoopTime;
  float byteReadRate;

  // compression -> medium  -> readspeed data
  std::map<int, std::map<std::string, ReadSpeedData>> readSpeedData;
  std::map<int, std::map<std::string, TGraphErrors *>> eventSpeedupGraphs;
  std::map<int, std::map<std::string, TGraphErrors *>> byteSpeedupGraphs;

  auto media = std::vector<std::string>{"ssd", "hdd", "tmpfs", "xrootd"};

  for (const int compression : {505, 404, 201, 207}) {
    for (const auto medium : media) {
      std::string ttreeResultsFilePath = std::string(resultsPathBase) + "/" + medium +
                                         "/ttree/readspeed_" + std::string(physFileType) + "_" +
                                         std::to_string(compression) + ".data";
      std::string rntupleResultsFilePath = std::string(resultsPathBase) + "/" + medium +
                                           "/rntuple_uring/readspeed_" + std::string(physFileType) +
                                           "_" + std::to_string(compression) + ".data";

      if (!std::filesystem::exists(rntupleResultsFilePath)) {
        rntupleResultsFilePath = std::string(resultsPathBase) + "/" + medium +
                                 "/rntuple/readspeed_" + std::string(physFileType) + "_" +
                                 std::to_string(compression) + ".data";
      }

      std::cout << "** Reading from " << ttreeResultsFilePath << std::endl;
      std::cout << "** Reading from " << rntupleResultsFilePath << std::endl;

      std::ifstream ttreeResultsFile(ttreeResultsFilePath);
      std::ifstream rntupleResultsFile(rntupleResultsFilePath);

      std::vector<float> ttreeLoopTimes;
      std::vector<float> ttreeReadRates;

      std::vector<float> rntupleLoopTimes;
      std::vector<float> rntupleReadRates;

      while (ttreeResultsFile >> eventLoopTime >> byteReadRate) {
        ttreeLoopTimes.push_back(eventLoopTime);
        ttreeReadRates.push_back(byteReadRate);
      }

      while (rntupleResultsFile >> eventLoopTime >> byteReadRate) {
        rntupleLoopTimes.push_back(eventLoopTime);
        rntupleReadRates.push_back(byteReadRate);
      }

      std::cout << "* Event speedup" << std::endl;

      RVec<float> ttreeLoopTimesVec(ttreeLoopTimes.begin(), ttreeLoopTimes.end());
      RVec<float> rntupleLoopTimesVec(rntupleLoopTimes.begin(), rntupleLoopTimes.end());
      auto eventSpeedupVec = (nEvents / rntupleLoopTimesVec) / (nEvents / ttreeLoopTimesVec);

      float eventSpeedupMedian = TMath::Median(eventSpeedupVec.size(), eventSpeedupVec.data());

      auto filteredEventSpeedup = Filter(eventSpeedupVec, [eventSpeedupMedian](float speedup) {
        return speedup <= eventSpeedupMedian + (eventSpeedupMedian * 0.5) &&
               speedup >= eventSpeedupMedian - (eventSpeedupMedian * 0.5);
      });

      std::cout << "\tRemoved " << eventSpeedupVec.size() - filteredEventSpeedup.size() << " (of "
                << eventSpeedupVec.size() << " total measurements) speedup outliers" << std::endl;

      float eventSpeedupMean = Mean(filteredEventSpeedup);
      float eventSpeedupErr = StdErr(filteredEventSpeedup);

      std::cout << "\tMean = " << eventSpeedupMean << "\tError = " << eventSpeedupErr << std::endl;

      std::cout << "* Byte speedup" << std::endl;

      RVec<float> ttreeReadRatesVec(ttreeReadRates.begin(), ttreeReadRates.end());
      RVec<float> rntupleReadRatesVec(rntupleReadRates.begin(), rntupleReadRates.end());
      auto byteSpeedupVec = rntupleReadRatesVec / ttreeReadRatesVec;

      float byteSpeedupMedian = TMath::Median(byteSpeedupVec.size(), byteSpeedupVec.data());

      auto filteredByteSpeedup = Filter(byteSpeedupVec, [byteSpeedupMedian](float readRate) {
        return readRate <= byteSpeedupMedian + (byteSpeedupMedian * 0.5) &&
               readRate >= byteSpeedupMedian - (byteSpeedupMedian * 0.5);
      });

      std::cout << "\tRemoved " << byteSpeedupVec.size() - filteredByteSpeedup.size() << " (of "
                << byteSpeedupVec.size() << " total measurements) throughput outliers "
                << std::endl;

      float byteSpeedupMean = Mean(filteredByteSpeedup);
      float byteSpeedupErr = StdErr(filteredByteSpeedup);

      std::cout << "\tMean = " << byteSpeedupMean << "\tError = " << byteSpeedupErr << std::endl;

      readSpeedData[compression][medium] =
          ReadSpeedData{eventSpeedupMean, eventSpeedupErr, byteSpeedupMean, byteSpeedupErr};
    }

    readSpeedData[compression]["filler"] = ReadSpeedData{0., 0., 0., 0.};
  }

  float maxEventSpeedup = 0.;
  float maxByteSpeedup = 0.;
  for (auto &[compression, media] : readSpeedData) {
    for (auto &[medium, data] : media) {
      int x;
      if (medium == "filler")
        x = 0;
      if (medium == "ssd")
        x = 1;
      else if (medium == "hdd")
        x = 2;
      else if (medium == "tmpfs")
        x = 3;
      else if (medium == "xrootd")
        x = 4;

      switch (compression) {
      case 505:
        x += 0;
        break;
      case 404:
        x += 5;
        break;
      case 201:
        x += 10;
        break;
      case 207:
        x += 15;
        break;
      }

      eventSpeedupGraphs[compression][medium] = new TGraphErrors();
      float eventSpeedupMean = data.eventSpeedupMean;
      float eventSpeedupErr = data.eventSpeedupErr;
      eventSpeedupGraphs[compression][medium]->SetPoint(0, x + 0, eventSpeedupMean);
      eventSpeedupGraphs[compression][medium]->SetPoint(1, x + 2, -1);
      eventSpeedupGraphs[compression][medium]->SetPointError(0, 0, eventSpeedupErr);
      maxEventSpeedup = std::max(maxEventSpeedup, eventSpeedupMean + eventSpeedupErr);

      byteSpeedupGraphs[compression][medium] = new TGraphErrors();
      float byteSpeedupMean = data.byteSpeedupMean;
      float byteSpeedupErr = data.byteSpeedupErr;
      byteSpeedupGraphs[compression][medium]->SetPoint(0, x + 0, byteSpeedupMean);
      byteSpeedupGraphs[compression][medium]->SetPoint(1, x + 2, -1);
      byteSpeedupGraphs[compression][medium]->SetPointError(0, 0, byteSpeedupErr);
      maxByteSpeedup = std::max(maxByteSpeedup, byteSpeedupMean + byteSpeedupErr);
    }
  }

  drawGraph(eventSpeedupGraphs, maxEventSpeedup, "event", save);
  drawGraph(byteSpeedupGraphs, maxByteSpeedup * 1.1, "byte", save);
}

void plot_throughput_ratio() {
  SetStyle();

  plot("results/chep-proc", "data", false);
}
