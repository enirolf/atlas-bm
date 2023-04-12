#include "plot_util.C"

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

void makePlot(const std::string &physFileType = "data", bool save = true) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//
  std::uint64_t nEvents;
  std::uint32_t nCols;
  float initTime;
  float loopTime;

  std::vector<std::string> formats;
  // compression -> storage format -> TGraphErrors*
  std::map<std::string, std::map<std::string, TGraphErrors *>> speedGraphMap;
  // compression -> storage format -> (mean, error)
  std::map<std::string, std::map<std::string, std::pair<float, float>>> dataMap;
  // compression -> TGraphErrors*
  std::map<std::string, TGraphErrors *> ratioGraphMap;

  float maxThroughput = 0.;
  for (const std::string compression : {"none", "zstd"}) {
    for (const std::string format : {"ttree", "rntuple"}) {
      formats.push_back(format);

      std::string resultsFilePath =
          "results/readspeed_cold_" + format + "_" + physFileType + "_" + compression + ".txt";

      std::ifstream resultsFile(resultsFilePath);
      std::vector<float> loopTimes;

      while (resultsFile >> nEvents >> nCols >> initTime >> loopTime) {
        float seconds = loopTime / 1000;
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
      int x = (compression == "zstd") * 2 + (format == "rntuple");
      g->SetPoint(0, x + 0.5, throughputMean);
      g->SetPoint(1, x + 2, -1);
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
    int x = (compression == "zstd");
    g->SetPoint(0, 2 * x + 0, -1);
    g->SetPoint(1, 2 * x + 1.5, ratioMean);
    g->SetPoint(2, 2 * x + 3, -1);
    g->SetPointError(1, 0, ratioErr);
    ratioGraphMap[compression] = g;
  }

  //--------------------------------------------------------------------------//
  // SETUP THE CANVAS                                                         //
  //--------------------------------------------------------------------------//

  TCanvas *canvas = new TCanvas(Form("canvas_%s", physFileType.c_str()), "canvas");
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
  // padTitle->SetBorderSize(1);
  auto padCenter = padTitle->GetBBoxCenter();
  auto title =
      new TText(0.5, 0.5, Form("TTree vs. RNTuple READ throughput (%s)", physFileType.c_str()));
  title->SetBBoxCenter(padTitle->GetBBoxCenter());
  title->SetTextColor(kBlack);
  title->SetTextSize(.9);
  title->SetTextAlign(23);
  title->Draw();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN GRAPH                                                      //
  //--------------------------------------------------------------------------//

  TH1F *helperSpeed = new TH1F("", "", 4, 0, 4);
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

  for (unsigned i = 2; i < 4; i += 2) {
    TLine *line = new TLine(i, 0, i, maxThroughput * 1.1);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  //--------------------------------------------------------------------------//
  // DRAW THE RATIO GRAPH                                                     //
  //--------------------------------------------------------------------------//

  TH1F *helperRatio = new TH1F("", "", 4, 0.5, 4.5);
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
    if (i == 3) {
      helperRatio->GetXaxis()->ChangeLabel(i, -1, 0.11, -1, -1, -1, "none");
    } else if (i == 7) {
      helperRatio->GetXaxis()->ChangeLabel(i, -1, 0.11, -1, -1, -1, "zstd");
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
    graph->SetFillStyle(styles.at(compression));
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

  TLine *lineOne = new TLine(0.5, 1, 4.5, 1);
  lineOne->SetLineColor(kBlack);
  lineOne->SetLineStyle(1);
  lineOne->SetLineWidth(1);
  lineOne->Draw();

  for (unsigned i = 2; i < 4; i += 2) {
    TLine *line = new TLine(i + 0.5, 0, i + 0.5, maxRatio * 1.1);
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
  leg->AddEntry(speedGraphMap["zstd"]["ttree"], "TTree", "F");
  leg->AddEntry(speedGraphMap["zstd"]["rntuple"], "RNTuple", "F");
  leg->AddEntry(ratioGraphMap["zstd"], "RNTuple / TTree storage ratio", "F");
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
    canvas->Print(Form("figures/readspeed_ssd_%s.pdf", physFileType.c_str()));
    canvas->Print(Form("figures/readspeed_ssd_%s.png", physFileType.c_str()));
  }
}

void plot_readspeed() {
  SetStyle();
  makePlot("data");
  makePlot("mc");
}
