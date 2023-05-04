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

void plot(std::string_view resultsPathBase, std::string_view physFileType = "data",
          bool save = false) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//
  float nEvents = (physFileType == "data" ? 209062 : 180000); // TODO add to results file
  float eventLoopTime;
  float byteReadRate;

  // compression -> medium  -> readspeed data
  std::map<int, std::map<std::string, ReadSpeedData>> readSpeedData;
  std::map<int, std::map<std::string, TGraphErrors *>> eventSpeedupGraphs;
  std::map<int, std::map<std::string, TGraphErrors *>> byteSpeedupGraphs;

  auto media = std::vector<std::string>{"ssd", "hdd", "tmpfs", "xrootd"};

  for (const int compression : {505, 201, 207}) {
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
      case 201:
        x += 5;
        break;
      case 207:
        x += 10;
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

  //==========================================================================//
  // EVENT SPEEDUP                                                            //
  //==========================================================================//

  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//

  TCanvas *canvasEventSpeedup =
      new TCanvas("canvas_event_speedup_overview", "canvas_event_speedup_overview", 1200, 600);
  canvasEventSpeedup->cd();

  // TTree vs RNTuple read throughput speed
  auto padEventSpeedup = new TPad("padEventSpeedup", "padEventSpeedup", 0.0, 0.0, 1.0, 1.0);
  padEventSpeedup->SetTopMargin(0.055);
  padEventSpeedup->SetBottomMargin(0.08);
  padEventSpeedup->SetLeftMargin(0.06);
  padEventSpeedup->SetRightMargin(0.02);
  padEventSpeedup->SetFillStyle(4000);
  padEventSpeedup->SetFrameFillStyle(4000);
  padEventSpeedup->Draw();
  canvasEventSpeedup->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//
  int nBins = 15;
  int binStart = 6;
  int binInterval = 10;

  maxEventSpeedup *= 1.1;

  TH1F *helperEventSpeedup = new TH1F("", "", nBins, 0, nBins);
  helperEventSpeedup->GetXaxis()->SetTickSize(0);
  helperEventSpeedup->GetXaxis()->SetNdivisions(nBins * 2);
  helperEventSpeedup->GetXaxis()->SetLabelOffset(0.01);
  helperEventSpeedup->GetYaxis()->SetTickSize(0.01);
  helperEventSpeedup->GetYaxis()->SetLabelSize(0.035);
  helperEventSpeedup->GetYaxis()->SetTitle("Throughput ratio (RNTuple / TTree)");
  helperEventSpeedup->GetYaxis()->SetTitleSize(0.035);
  helperEventSpeedup->GetYaxis()->SetTitleOffset(0.7);
  helperEventSpeedup->SetMinimum(0);
  helperEventSpeedup->SetMaximum(maxEventSpeedup);

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helperEventSpeedup->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "zstd");
    } else if (i == binStart + binInterval) {
      helperEventSpeedup->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 1)");
    } else if (i == binStart + (2 * binInterval)) {
      helperEventSpeedup->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 7)");
    } else {
      helperEventSpeedup->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  padEventSpeedup->cd();
  gPad->SetGridy();

  helperEventSpeedup->Draw();

  for (const auto &[compression, media] : readSpeedData) {
    for (const auto &[medium, data] : media) {
      eventSpeedupGraphs[compression][medium]->SetLineColor(12);
      eventSpeedupGraphs[compression][medium]->SetMarkerColor(12);
      eventSpeedupGraphs[compression][medium]->SetFillColor(colors.at(medium));
      eventSpeedupGraphs[compression][medium]->SetLineWidth(2);
      eventSpeedupGraphs[compression][medium]->Draw("B1");
      eventSpeedupGraphs[compression][medium]->Draw("P");

      for (int i = 0; i < eventSpeedupGraphs[compression][medium]->GetN(); ++i) {

        double x, y;
        eventSpeedupGraphs[compression][medium]->GetPoint(i, x, y);

        if (y < 0)
          continue;

        std::ostringstream val;
        val.precision(1);
        val << "#times" << std::fixed << y;

        TLatex tval;
        tval.SetTextColor(kWhite);
        tval.SetTextSize(0.03);
        tval.SetTextAlign(21);
        tval.DrawLatex(x, maxEventSpeedup * 0.025, val.str().c_str());
      }
    }
  }

  for (unsigned i = 0; i < nBins; i += (binInterval / 2)) {
    TLine *line = new TLine(i, 0, i, maxEventSpeedup);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *leg = new TLegend(0.775, 0.8, 0.975, 0.935);
  leg->AddEntry(eventSpeedupGraphs[505]["ssd"], "SSD", "F");
  leg->AddEntry(eventSpeedupGraphs[505]["hdd"], "HDD", "F");
  leg->AddEntry(eventSpeedupGraphs[505]["tmpfs"], "RAM", "F");
  leg->AddEntry(eventSpeedupGraphs[505]["xrootd"], "XRootD (100GbE, 0.3ms)", "F");
  leg->SetNColumns(1);
  leg->SetMargin(0.15);
  leg->Draw();

  TText l;
  l.SetTextSize(0.025);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.935, 0.785, "95% CL");

  TLine *lineOne = new TLine(0, 1, 15, 1);
  lineOne->SetLineColor(kBlack);
  lineOne->SetLineStyle(7);
  lineOne->SetLineWidth(2);
  lineOne->Draw();

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvasEventSpeedup->Print("figures/readspeed_event_speedup.pdf");
    canvasEventSpeedup->Print("figures/readspeed_event_speedup.png");
  }

  //==========================================================================//
  // BYTE SPEEDUP                                                             //
  //==========================================================================//

  //--------------------------------------------------------------------------//
  // SET UP THE CANVAS                                                        //
  //--------------------------------------------------------------------------//

  TCanvas *canvasByteSpeedup =
      new TCanvas("canvas_byte_speedup_overview", "canvas_byte_speedup_overview", 1200, 600);
  canvasByteSpeedup->cd();

  // TTree vs RNTuple read throughput speed
  auto padByteSpeedup = new TPad("padByteSpeedup", "padByteSpeedup", 0.0, 0.0, 1.0, 1.0);
  padByteSpeedup->SetTopMargin(0.055);
  padByteSpeedup->SetBottomMargin(0.08);
  padByteSpeedup->SetLeftMargin(0.06);
  padByteSpeedup->SetRightMargin(0.02);
  padByteSpeedup->SetFillStyle(4000);
  padByteSpeedup->SetFrameFillStyle(4000);
  padByteSpeedup->Draw();
  canvasByteSpeedup->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//

  maxByteSpeedup *= 1.1;

  TH1F *helperBytesSpeedup = new TH1F("", "", nBins, 0, nBins);
  helperBytesSpeedup->GetXaxis()->SetTickSize(0);
  helperBytesSpeedup->GetXaxis()->SetNdivisions(nBins * 2);
  helperBytesSpeedup->GetXaxis()->SetLabelOffset(0.01);
  helperBytesSpeedup->GetYaxis()->SetTickSize(0.01);
  helperBytesSpeedup->GetYaxis()->SetLabelSize(0.035);
  helperBytesSpeedup->GetYaxis()->SetTitle("Speedup (RNTuple / TTree)");
  helperBytesSpeedup->GetYaxis()->SetTitleSize(0.035);
  helperBytesSpeedup->GetYaxis()->SetTitleOffset(0.7);
  helperBytesSpeedup->SetMinimum(0);
  helperBytesSpeedup->SetMaximum(maxByteSpeedup);

  for (int i = 0; i <= nBins * 2 + 1; i++) {
    if (i == binStart) {
      helperBytesSpeedup->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "zstd");
    } else if (i == binStart + binInterval) {
      helperBytesSpeedup->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 1)");
    } else if (i == binStart + (2 * binInterval)) {
      helperBytesSpeedup->GetXaxis()->ChangeLabel(i, -1, 0.035, 21, -1, -1, "lzma (lvl 7)");
    } else {
      helperBytesSpeedup->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  padByteSpeedup->cd();
  gPad->SetGridy();

  helperBytesSpeedup->Draw();

  for (const auto &[compression, media] : readSpeedData) {
    for (const auto &[medium, data] : media) {
      byteSpeedupGraphs[compression][medium]->SetLineColor(12);
      byteSpeedupGraphs[compression][medium]->SetMarkerColor(12);
      byteSpeedupGraphs[compression][medium]->SetFillColor(colors.at(medium));
      byteSpeedupGraphs[compression][medium]->SetLineWidth(2);
      byteSpeedupGraphs[compression][medium]->Draw("B1");
      byteSpeedupGraphs[compression][medium]->Draw("P");

      for (int i = 0; i < byteSpeedupGraphs[compression][medium]->GetN(); ++i) {

        double x, y;
        byteSpeedupGraphs[compression][medium]->GetPoint(i, x, y);

        if (y < 0)
          continue;

        std::ostringstream val;
        val.precision(1);
        val << "#times" << std::fixed << y;

        TLatex tval;
        tval.SetTextColor(kWhite);
        tval.SetTextSize(0.03);
        tval.SetTextAlign(21);
        tval.DrawLatex(x, maxEventSpeedup * 0.025, val.str().c_str());
      }
    }
  }

  for (unsigned i = 0; i < nBins; i += (binInterval / 2)) {
    TLine *line = new TLine(i, 0, i, maxByteSpeedup);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *legByte = new TLegend(0.775, 0.8, 0.975, 0.935);
  legByte->AddEntry(byteSpeedupGraphs[505]["ssd"], "SSD", "F");
  legByte->AddEntry(byteSpeedupGraphs[505]["hdd"], "HDD", "F");
  legByte->AddEntry(byteSpeedupGraphs[505]["tmpfs"], "RAM", "F");
  legByte->AddEntry(byteSpeedupGraphs[505]["xrootd"], "XRootD (100GbE, 0.3ms)", "F");
  legByte->SetNColumns(1);
  legByte->SetMargin(0.15);
  legByte->Draw();

  // TText l;
  l.SetTextSize(0.025);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.935, 0.785, "95% CL");

  // TLine *lineOne = new TLine(0, 1, 15, 1);
  lineOne->SetLineColor(kBlack);
  lineOne->SetLineStyle(7);
  lineOne->SetLineWidth(2);
  lineOne->Draw();

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvasByteSpeedup->Print("figures/readspeed_byte_speedup.pdf");
    canvasByteSpeedup->Print("figures/readspeed_byte_speedup.png");
  }
}

void plot_speedup() {
  SetStyle();

  plot("results/chep", "data", true);
}
