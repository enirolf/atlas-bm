#include "../bm-utils/plot_util.C"

#include <cmath>
#include <filesystem>
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

void drawGraph(std::map<int, std::map<std::string, TGraphErrors *>> &graphs, float maxThroughput,
               std::string_view plotKind, std::string_view physFileType, bool save = false) {
  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//
  TCanvas *canvas =
      new TCanvas(Form("canvas_througput_%s_%s", std::string(plotKind).c_str(), std::string(physFileType).c_str()),
                  Form("canvas_througput_%s_%s", std::string(plotKind).c_str(), std::string(physFileType).c_str()), 1200, 500);
  canvas->SetFillStyle(4000);
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto pad = new TPad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad->SetTopMargin(0.085);
  pad->SetBottomMargin(0.18);
  pad->SetLeftMargin(0.06);
  pad->SetRightMargin(0.005);
  pad->SetFillStyle(4000);
  pad->SetFrameFillStyle(4000);
  pad->SetFillStyle(4000);
  pad->Draw();
  canvas->cd();

  int nBins = 20;
  int binStart = 6;
  int binInterval = 10;

  //--------------------------------------------------------------------------//
  // DRAW THE GRAPH                                                           //
  //--------------------------------------------------------------------------//
  maxThroughput *= 1.1;

  TH1F *helper = new TH1F("", "", nBins, 0, nBins);
  helper->GetXaxis()->SetTitle("Compression method");
  helper->GetXaxis()->SetTitleSize(0.05);
  helper->GetXaxis()->SetTitleOffset(1.45);
  helper->GetXaxis()->SetTickSize(0);
  helper->GetXaxis()->SetNdivisions(nBins * 2);
  helper->GetXaxis()->SetLabelOffset(0.03);
  helper->GetYaxis()->SetTickSize(0.01);
  helper->GetYaxis()->SetLabelSize(0.045);
  helper->GetYaxis()->SetTitle("Throughput ratio (RNTuple / TTree)");
  helper->GetYaxis()->SetTitleSize(0.05);
  helper->GetYaxis()->SetTitleOffset(0.5);
  helper->SetMinimum(0);
  helper->SetMaximum(maxThroughput);

  float labelSize = 0.05;

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "lz4");
    } else if (i == binStart + binInterval) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "zstd*");
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
        tval.SetTextSize(0.05);
        tval.SetTextAlign(21);
        if (y < 0.7) {
          tval.SetTextColor(kBlack);
          tval.DrawLatex(x, maxThroughput * 0.11, val.str().c_str());
        } else {
          tval.SetTextColor(kWhite);
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

  TLegend *leg = new TLegend(0.8, 0.7, 0.985, 0.9);
  leg->AddEntry(graphs[505]["ssd"], "SSD", "F");
  leg->AddEntry(graphs[505]["hdd"], "HDD", "F");
  leg->AddEntry(graphs[505]["tmpfs"], "RAM", "F");
  leg->AddEntry(graphs[505]["xrootd"], "XRootD (100GbE, 0.3ms)", "F");
  leg->SetNColumns(1);
  leg->Draw();

  TText clAnnot;
  clAnnot.SetTextSize(0.04);
  clAnnot.SetTextAlign(33);
  clAnnot.SetTextFont(42);
  clAnnot.DrawTextNDC(0.995, 0.96, "95% CL");

  TText zstdAnnot;
  zstdAnnot.SetTextSize(0.035);
  zstdAnnot.SetTextAlign(33);
  zstdAnnot.SetTextFont(42);
  zstdAnnot.DrawTextNDC(0.995, 0.035, "*current ATLAS default");

  TLine *lineOne = new TLine(0, 1, 20, 1);
  lineOne->SetLineColor(kBlack);
  lineOne->SetLineStyle(7);
  lineOne->SetLineWidth(1);
  lineOne->Draw();

  TLatex title;
  title.SetTextSize(0.05);
  title.SetTextAlign(23);
  title.SetTextFont(42);
  if (physFileType == "mc")
    title.DrawLatexNDC(0.5, 0.975, "DAOD_PHYS event throughput ratio of RNTuple wrt. TTree, MC");
  else
    title.DrawLatexNDC(0.5, 0.975, "DAOD_PHYS event throughput ratio of RNTuple wrt. TTree, data");

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//
  if (save) {
    canvas->Print(Form("figures/readspeed_throughput_ratio_%s_%s.pdf", std::string(plotKind).c_str(), std::string(physFileType).c_str()));
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

  for (const int compression : {404, 505, 201, 207}) {
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
      case 404:
        x += 0;
        break;
      case 505:
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

  drawGraph(eventSpeedupGraphs, maxEventSpeedup, "event", physFileType, save);
}

void plot_throughput_ratio() {
  SetStyle();

  plot("results", "data", true);
  plot("results", "mc", true);
}
