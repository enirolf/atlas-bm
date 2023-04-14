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

void makePlot(std::string_view resultsPathBase, std::string_view medium,
              std::string_view physFileType = "data", bool save = true) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//
  float nEvents;
  float initTime;
  float loopTime;

  // compression -> storage format -> TGraphErrors*
  std::map<int, std::map<std::string, TGraphErrors *>> speedGraphMap;
  // compression -> storage format -> (mean, error)
  std::map<int, std::map<std::string, std::pair<float, float>>> dataMap;
  // compression -> TGraphErrors*
  std::map<int, TGraphErrors *> ratioGraphMap;

  float maxThroughput = 0.;
  for (const int compression : {0, 505, 201, 207}) {
    for (const std::string format : {"ttree", "rntuple"}) {

      std::string resultsFilePath = std::string(resultsPathBase) + "/readspeed_cold_" + format +
                                    "_" + std::string(physFileType) + "_" +
                                    std::to_string(compression) + ".txt";

      std::ifstream resultsFile(resultsFilePath);
      std::vector<float> loopTimes;

      while (resultsFile >> nEvents >> initTime >> loopTime) {
        float seconds = loopTime / 1e6;
        loopTimes.push_back(seconds);
      }

      RVec<float> loopTimeVec(loopTimes.begin(), loopTimes.end());
      float secondsMean = Mean(loopTimeVec);
      float secondsErr = StdErr(loopTimeVec);
      float throughputMean = nEvents / secondsMean;
      float throughputMax = nEvents / (secondsMean - secondsErr);
      float throughputMin = nEvents / (secondsMean + secondsErr);
      float throughputErr = (throughputMax - throughputMin) / 2.;

      dataMap[compression][format] = std::pair<float, float>(throughputMean, throughputErr);

      auto g = new TGraphErrors();
      int x = (format == "rntuple");
      switch (compression) {
      case 0:
        x += 0;
        break;
      case 505:
        x += 2;
        break;
      case 201:
        x += 4;
        break;
      case 207:
        x += 6;
        break;
      }
      g->SetPoint(0, x + 0.5, throughputMean);
      g->SetPoint(1, x + 2.25, -1);
      g->SetPointError(0, 0, throughputErr);
      speedGraphMap[compression][format] = g;
      maxThroughput = std::max(maxThroughput, throughputMean + throughputErr);
    }
  }

  float maxRatio = 0.;
  for (const auto &[compression, data] : dataMap) {
    float rntupleMean = data.at("rntuple").first;
    float rntupleErr = data.at("rntuple").second;
    float ttreeMean = data.at("ttree").first;
    float ttreeErr = data.at("ttree").second;
    float ratioMean = rntupleMean / ttreeMean;
    float ratioErr = ratioMean * std::sqrt(std::pow(ttreeErr, 2) / std::pow(ttreeMean, 2) +
                                           std::pow(rntupleErr, 2) / std::pow(rntupleMean, 2));

    maxRatio = std::max(maxRatio, ratioMean + ratioErr);

    auto g = new TGraphErrors();
    int x;
    switch (compression) {
    case 0:
      x = 0;
      break;
    case 505:
      x = 1;
      break;
    case 201:
      x = 2;
      break;
    case 207:
      x = 3;
      break;
    }
    g->SetPoint(0, 2 * x + 0, -1);
    g->SetPoint(1, 2 * x + 2, ratioMean);
    g->SetPoint(2, 2 * x + 4, -1);
    g->SetPointError(1, 0, ratioErr);
    ratioGraphMap[compression] = g;
  }

  //--------------------------------------------------------------------------//
  // SETUP THE CANVAS                                                         //
  //--------------------------------------------------------------------------//

  TCanvas *canvas = new TCanvas(
      Form("canvas__%s_%s", std::string(medium).c_str(), std::string(physFileType).c_str()),
      "canvas");
  canvas->cd();

  // Title pad
  auto padTitle = new TPad("padTitle", "padTitle", 0.0, 0.9, 1.0, 0.95);
  padTitle->SetTopMargin(0.08);
  padTitle->SetBottomMargin(0.03);
  padTitle->SetLeftMargin(0.1);
  padTitle->SetRightMargin(0.005);
  padTitle->Draw();
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto padSpeed = new TPad("padSpeed", "padSpeed", 0., 0.39, 1.0, 0.89);
  padSpeed->SetTopMargin(0.08);
  padSpeed->SetBottomMargin(0.03);
  padSpeed->SetLeftMargin(0.1);
  padSpeed->SetRightMargin(0.055);
  padSpeed->Draw();
  canvas->cd();

  // TTree vs RNTuple read throughput ratio
  auto padRatio = new TPad("padRatio", "padRatio", 0., 0.09, 1.0, 0.38);
  padRatio->SetTopMargin(0.05);
  padRatio->SetBottomMargin(0.26);
  padRatio->SetLeftMargin(0.1);
  padRatio->SetRightMargin(0.055);
  padRatio->Draw();
  canvas->cd();

  // Legend
  auto padLegend = new TPad("padLegend", "padLegend", 0.0, 0.03, 1.0, 0.08);
  padLegend->SetTopMargin(0.01);
  padLegend->SetBottomMargin(0.26);
  padLegend->SetLeftMargin(0.1);
  padLegend->SetRightMargin(0.055);
  padLegend->Draw();
  canvas->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE TITLE                                                           //
  //--------------------------------------------------------------------------//

  padTitle->cd();
  auto padCenter = padTitle->GetBBoxCenter();
  auto title = new TText(0.5, 0.5,
                         Form("TTree vs. RNTuple %s READ throughput (%s)",
                              std::string(medium).c_str(), std::string(physFileType).c_str()));
  title->SetBBoxCenter(padTitle->GetBBoxCenter());
  title->SetTextColor(kBlack);
  title->SetTextSize(.9);
  title->SetTextAlign(23);
  title->Draw();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//

  TH1F *helperSpeed = new TH1F("", "", 8, 0, 8);
  helperSpeed->GetXaxis()->SetTitle("");
  helperSpeed->GetXaxis()->SetNdivisions(0);
  helperSpeed->GetXaxis()->SetLabelSize(0);
  helperSpeed->GetXaxis()->SetTickSize(0);
  helperSpeed->GetYaxis()->SetTitle("Events / s");
  helperSpeed->GetYaxis()->SetTickSize(0.01);
  helperSpeed->GetYaxis()->SetLabelSize(0.07);
  helperSpeed->GetYaxis()->SetTitleSize(0.07);
  helperSpeed->GetYaxis()->SetTitleOffset(0.725);
  helperSpeed->SetMinimum(0);
  helperSpeed->SetMaximum(maxThroughput * 1.1);

  padSpeed->cd();
  gPad->SetGridy();

  helperSpeed->Draw();

  for (const auto &[compression, formats] : speedGraphMap) {
    for (const auto &[format, graph] : formats) {
      graph->SetLineColor(12);
      graph->SetMarkerColor(12);
      graph->SetFillColor(colors.at(format));
      graph->SetLineWidth(2);
      graph->Draw("B1");
      graph->Draw("P");
    }
  }

  for (unsigned i = 2; i < 8; i += 2) {
    TLine *line = new TLine(i, 0, i, maxThroughput * 1.1);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  //--------------------------------------------------------------------------//
  // DRAW THE RATIO GRAPH                                                     //
  //--------------------------------------------------------------------------//

  TH1F *helperRatio = new TH1F("", "", 8, 1, 9);
  helperRatio->SetMinimum(0);
  helperRatio->SetMaximum(maxRatio * 1.1);
  helperRatio->GetXaxis()->SetTickSize(0);
  helperRatio->GetXaxis()->SetNdivisions(8);
  helperRatio->GetXaxis()->SetLabelSize(0.16);
  helperRatio->GetXaxis()->SetLabelOffset(0.01);
  helperRatio->GetXaxis()->SetTitleSize(0.12);
  helperRatio->GetYaxis()->SetTitle("RNTuple / TTree");
  helperRatio->GetYaxis()->SetTickSize(0.005);
  helperRatio->GetYaxis()->SetNdivisions(8);
  helperRatio->GetYaxis()->SetLabelSize(0.11);
  helperRatio->GetYaxis()->SetTitleSize(0.11);
  helperRatio->GetYaxis()->SetTitleOffset(0.45);

  for (int i = 0; i <= helperRatio->GetXaxis()->GetNlabels(); i++) {
    if (i == 2) {
      helperRatio->GetXaxis()->ChangeLabel(i, -1, 0.095, -1, -1, -1, "no compression");
    } else if (i == 4) {
      helperRatio->GetXaxis()->ChangeLabel(i, -1, 0.095, -1, -1, -1, "zstd");
    } else if (i == 6) {
      helperRatio->GetXaxis()->ChangeLabel(i, -1, 0.095, -1, -1, -1, "lzma (lvl 1)");
    } else if (i == 8) {
      helperRatio->GetXaxis()->ChangeLabel(i, -1, 0.095, -1, -1, -1, "lzma (lvl 7)");
    } else {
      helperRatio->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  padRatio->cd();
  gPad->SetGridy();

  helperRatio->Draw();

  for (const auto &[compression, graph] : ratioGraphMap) {
    graph->SetLineColor(12);
    graph->SetMarkerColor(12);
    graph->SetFillColor(kPlotGreen);
    graph->SetFillStyle(1001);
    graph->SetLineWidth(2);
    graph->Draw("B1");
    graph->Draw("P");

    double x, y, err;
    graph->GetPoint(1, x, y);
    err = graph->GetErrorY(1);
    std::ostringstream val;
    val.precision(1);
    val << "#times" << std::fixed << y;
    val << " #pm ";
    val << std::fixed << err;

    TLatex tval;
    tval.SetTextSize(0.08);
    tval.SetTextAlign(23);
    tval.DrawLatex(x, y * 0.8, val.str().c_str());
  }

  TLine *lineOne = new TLine(1, 1, 9, 1);
  lineOne->SetLineColor(kBlack);
  lineOne->SetLineStyle(1);
  lineOne->SetLineWidth(1);
  lineOne->Draw();

  for (unsigned i = 2; i < 8; i += 2) {
    TLine *line = new TLine(i + 1, 0, i + 1, maxRatio * 1.1);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  //--------------------------------------------------------------------------//
  // DRAW THE LEGEND                                                          //
  //--------------------------------------------------------------------------//

  padLegend->cd();

  TLegend *leg = new TLegend(0.2, 0.1, 0.8, 0.9);
  leg->AddEntry(speedGraphMap[505]["ttree"], "TTree", "F");
  leg->AddEntry(speedGraphMap[505]["rntuple"], "RNTuple", "F");
  leg->AddEntry(ratioGraphMap[505], "RNTuple / TTree throughput ratio", "F");
  leg->SetNColumns(3);
  leg->SetColumnSeparation(.05);
  leg->SetBorderSize(0);
  leg->SetTextSize(0.65);
  leg->Draw();

  TText l;
  l.SetTextSize(0.04);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.175, 0.68 - 0.0075, "95% CL");

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvas->Print(Form("figures/readspeed_%s_%s.pdf", std::string(medium).c_str(),
                       std::string(physFileType).c_str()));
    canvas->Print(Form("figures/readspeed_%s_%s.png", std::string(medium).c_str(),
                       std::string(physFileType).c_str()));
  }
}

void makeOverviewPlot(std::string_view resultsPathBase, std::string_view medium,
                      std::string_view physFileType = "data", bool save = true) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//
  float nEvents;
  float initTime;
  float loopTime;

  // compression -> storage format -> TGraphErrors*
  std::map<int, std::map<std::string, TGraphErrors *>> speedGraphMap;
  // compression -> storage format -> (mean, error)
  std::map<int, std::map<std::string, std::pair<float, float>>> dataMap;
  // compression -> TGraphErrors*
  std::map<int, TGraphErrors *> ratioGraphMap;

  for (const int compression : {0, 505, 201, 207}) {
    for (const std::string format :
         {"ttree", "rntuple", "rntuple_mt", "rntuple_uring", "rntuple_mt_uring"}) {
      std::string resultsFilePath = std::string(resultsPathBase) + "/" + format +
                                    "/readspeed_cold_" + std::string(physFileType) + "_" +
                                    std::to_string(compression) + ".txt";

      std::ifstream resultsFile(resultsFilePath);

      std::vector<float> loopTimes;
      while (resultsFile >> nEvents >> initTime >> loopTime) {
        float seconds = loopTime / 1e6;
        loopTimes.push_back(seconds);
      }

      RVec<float> loopTimeVec(loopTimes.begin(), loopTimes.end());
      float secondsMean = Mean(loopTimeVec);
      float secondsErr = StdErr(loopTimeVec);
      float throughputMean = nEvents / secondsMean;
      float throughputMax = nEvents / (secondsMean - secondsErr);
      float throughputMin = nEvents / (secondsMean + secondsErr);
      float throughputErr = (throughputMax - throughputMin) / 2.;

      dataMap[compression][format] = std::pair<float, float>(throughputMean, throughputErr);
    }
    dataMap[compression]["fill"] = std::pair<float, float>(0., 0.);
  }

  float maxThroughput = 0.;
  for (const auto &[compression, formats] : dataMap) {
    for (const auto &[format, data] : formats) {
      auto g = new TGraphErrors();
      int x;
      if (format == "filler")
        x = 0;
      if (format == "ttree")
        x = 1;
      else if (format == "rntuple")
        x = 2;
      else if (format == "rntuple_mt")
        x = 3;
      else if (format == "rntuple_uring")
        x = 4;
      else if (format == "rntuple_mt_uring")
        x = 5;

      switch (compression) {
      case 0:
        x += 0;
        break;
      case 505:
        x += 6;
        break;
      case 201:
        x += 12;
        break;
      case 207:
        x += 18;
        break;
      }
      g->SetPoint(0, x + 0, data.first);
      g->SetPoint(1, x + 2, -1);
      g->SetPointError(0, 0, data.second);
      speedGraphMap[compression][format] = g;
      maxThroughput = std::max(maxThroughput, data.first + data.second);
    }
  }

  //--------------------------------------------------------------------------//
  // SETUP THE CANVAS                                                         //
  //--------------------------------------------------------------------------//

  TCanvas *canvas = new TCanvas(
      Form("canvas__%s_%s", std::string(medium).c_str(), std::string(physFileType).c_str()),
      "canvas", 1200, 600);
  canvas->cd();

  // Title pad
  auto padTitle = new TPad("padTitle", "padTitle", 0.0, 0.93, 1.0, 0.97);
  padTitle->SetTopMargin(0.02);
  padTitle->SetBottomMargin(0.01);
  padTitle->SetLeftMargin(0.1);
  padTitle->SetRightMargin(0.005);
  padTitle->SetFillStyle(4000);
  padTitle->SetFrameFillStyle(4000);
  padTitle->Draw();
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto padSpeed = new TPad("padSpeed", "padSpeed", 0.0, 0.03, 1.0, 0.92);
  padSpeed->SetTopMargin(0.055);
  padSpeed->SetBottomMargin(0.08);
  padSpeed->SetLeftMargin(0.08);
  padSpeed->SetRightMargin(0.04);
  padSpeed->SetFillStyle(4000);
  padSpeed->SetFrameFillStyle(4000);
  padSpeed->Draw();
  canvas->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE TITLE                                                           //
  //--------------------------------------------------------------------------//

  padTitle->cd();
  auto padCenter = padTitle->GetBBoxCenter();
  auto title = new TText(0.5, 0.5,
                         Form("TTree vs. RNTuple %s read throughput (%s)",
                              std::string(medium).c_str(), std::string(physFileType).c_str()));
  title->SetBBoxCenter(padTitle->GetBBoxCenter());
  title->SetTextColor(kBlack);
  title->SetTextSize(1.00);
  title->SetTextAlign(23);
  title->Draw();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//

  TH1F *helperSpeed = new TH1F("", "", 24, 0, 24);
  helperSpeed->GetXaxis()->SetTickSize(0);
  helperSpeed->GetXaxis()->SetNdivisions(25);
  helperSpeed->GetXaxis()->SetLabelOffset(0.01);
  helperSpeed->GetXaxis()->SetTitleSize(0.12);
  helperSpeed->GetYaxis()->SetTitle("Events / s");
  helperSpeed->GetYaxis()->SetTickSize(0.01);
  helperSpeed->GetYaxis()->SetLabelSize(0.045);
  helperSpeed->GetYaxis()->SetTitleSize(0.05);
  helperSpeed->GetYaxis()->SetTitleOffset(0.725);
  helperSpeed->SetMinimum(0);
  helperSpeed->SetMaximum(maxThroughput * 1.25);

  for (int i = 0; i <= helperSpeed->GetXaxis()->GetNlabels(); i++) {
    if (i == 4) {
      helperSpeed->GetXaxis()->ChangeLabel(i, -1, 0.04, 21, -1, -1, "no compression");
    } else if (i == 10) {
      helperSpeed->GetXaxis()->ChangeLabel(i, -1, 0.04, 21, -1, -1, "zstd");
    } else if (i == 16) {
      helperSpeed->GetXaxis()->ChangeLabel(i, -1, 0.04, 21, -1, -1, "lzma (lvl 1)");
    } else if (i == 22) {
      helperSpeed->GetXaxis()->ChangeLabel(i, -1, 0.04, 21, -1, -1, "lzma (lvl 7)");
    } else {
      helperSpeed->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  padSpeed->cd();
  gPad->SetGridy();

  helperSpeed->Draw();

  for (const auto &[compression, formats] : speedGraphMap) {
    for (const auto &[format, graph] : formats) {
      graph->SetLineColor(12);
      graph->SetMarkerColor(12);
      graph->SetFillColor(colors.at(format));
      graph->SetFillStyle(styles.at(format));
      graph->SetLineWidth(2);
      graph->Draw("B1");
      graph->Draw("P");
    }
  }

  for (unsigned i = 6; i < 24; i += 6) {
    TLine *line = new TLine(i, 0, i, maxThroughput * 1.25);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *leg = new TLegend(0.75, 0.7, 0.955, 0.935);
  leg->AddEntry(speedGraphMap[505]["ttree"], "TTree", "F");
  leg->AddEntry(speedGraphMap[505]["rntuple"], "RNTuple", "F");
  leg->AddEntry(speedGraphMap[505]["rntuple_mt"], "RNTuple (w/ decomp. offloading)", "F");
  leg->AddEntry(speedGraphMap[505]["rntuple_uring"], "RNTuple (w/ liburing)", "F");
  leg->AddEntry(speedGraphMap[505]["rntuple_mt_uring"],
                "RNTuple (w/ decomp. offloading + liburing)", "F");
  leg->SetNColumns(1);
  leg->SetMargin(0.15);
  leg->Draw();

  TText l;
  l.SetTextSize(0.025);
  l.SetTextAlign(13);
  l.DrawTextNDC(0.918, 0.685, "95% CL");

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    canvas->Print(Form("figures/readspeed_overview_%s_%s.pdf", std::string(medium).c_str(),
                       std::string(physFileType).c_str()));
    canvas->Print(Form("figures/readspeed_overview_%s_%s.png", std::string(medium).c_str(),
                       std::string(physFileType).c_str()));
  }
}

void plot_readspeed() {
  SetStyle();
  // makePlot("results/ssd", "SSD", "data", false);
  // makePlot("results/ssd", "SSD", "mc", false);
  // makePlot("results/ssd_mt", "SSD+MT", "data", false);
  // makePlot("results/ssd_mt", "SSD+MT", "mc", false);
  makeOverviewPlot("results/ssd", "SSD", "data", true);
  makeOverviewPlot("results/ssd", "SSD", "mc", true);
}
