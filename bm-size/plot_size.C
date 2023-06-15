#include "../bm-utils/plot_util.C"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TPad.h>

void makePlot(const std::string &physFileType = "data", bool save = true) {
  //--------------------------------------------------------------------------//
  // PREPARE THE GRAPHS                                                       //
  //--------------------------------------------------------------------------//

  std::ifstream file("results/size_" + physFileType + ".txt");
  std::string format;
  int compression;
  float nEvents, nCols, onDiskSize, inMemorySize;

  std::vector<std::string> formats;
  // compression -> storage format -> TGraphErrors*
  std::map<int, std::map<std::string, TGraphErrors *>> sizeGraphMap;
  // compression -> storage format -> size/event
  std::map<int, std::map<std::string, float>> dataMap;
  // compression -> TGraphErrors*
  std::map<int, TGraphErrors *> ratioGraphMap;

  float maxInputSize = 0.;
  while (file >> format >> compression >> nEvents >> nCols >> onDiskSize >> inMemorySize) {
    float eventSize;

    if (compression == 0)
      continue;
      // eventSize = inMemorySize / nEvents;
    else
      eventSize = onDiskSize / nEvents;
    dataMap[compression][format] = eventSize / 1024;
    dataMap[compression]["filler"] = 0;
    maxInputSize = std::max(maxInputSize, eventSize);
  }

  float maxRatio = 0.;
  for (const auto &[compression, formats] : dataMap) {
    for (const auto &[format, data] : formats) {
      float eventSize = dataMap[compression][format];

      if (maxInputSize < 1024)
        dataMap[compression][format] *= 1024.;

      int x;
      if (format == "filler")
        x = 0;
      if (format == "ttree")
        x = 1;
      else if (format == "rntuple")
        x = 2;

      switch (compression) {
      case 0:
        x += 0;
        break;
      case 404:
        // x += 3;
        x += 0;
        break;
      case 505:
        // x += 6;
        x += 3;
        break;
      case 201:
        // x += 9;
        x += 6;
        break;
      case 207:
        // x += 12;
        x += 9;
        break;
      }

      auto g = new TGraphErrors();
      g->SetPoint(0, x + 0, eventSize);
      g->SetPoint(1, x + 2, -1);
      g->SetPointError(0, 0, 0);
      sizeGraphMap[compression][format] = g;
    }

    float ratio = dataMap[compression]["rntuple"] / dataMap[compression]["ttree"];
    maxRatio = std::max(maxRatio, ratio);

    int x;
    switch (compression) {
    case 0:
      x = 0;
      break;
    case 404:
      x = 0;
      // x = 1;
      break;
    case 505:
      x = 1;
      // x = 2;
      break;
    case 201:
      x = 2;
      // x = 3;
      break;
    case 207:
      x = 3;
      // x = 4;
      break;
    }

    auto g = new TGraphErrors();
    g->SetPoint(0, 2 * x + 0, -1);
    g->SetPoint(1, 2 * x + 2, ratio);
    g->SetPoint(2, 2 * x + 4, -1);
    g->SetPointError(1, 0, 0);
    ratioGraphMap[compression] = g;
  }

  float maxSize = maxInputSize;
  if (maxInputSize > 1024) {
    maxSize = maxInputSize / 1024;
  }

  //--------------------------------------------------------------------------//
  // SETUP THE CANVAS                                                         //
  //--------------------------------------------------------------------------//

  TCanvas *canvas = new TCanvas(Form("canvas_%s", physFileType.c_str()), "canvas", 1200);
  canvas->cd();
  canvas->SetFillStyle(4000);

  // TTree and RNTuple on-disk size (no comp. + zstd)
  auto padDiskSize = new TPad("padDiskSize", "padDiskSize", 0.0, 0.40, 1.0, 1.0);
  padDiskSize->SetTopMargin(0.03);
  padDiskSize->SetBottomMargin(0.05);
  padDiskSize->SetLeftMargin(0.095);
  padDiskSize->SetRightMargin(0.01);
  padDiskSize->SetFillStyle(4000);
  padDiskSize->SetFrameFillStyle(4000);
  padDiskSize->Draw();
  canvas->cd();

  // TTree vs RNTuple on-disk size ratio (no comp. + zstd)
  auto padDiskRatio = new TPad("padDiskRatio", "padDiskRatio", 0.0, 0.0, 1.0, 0.39);
  padDiskRatio->SetTopMargin(0.05);
  padDiskRatio->SetBottomMargin(0.2);
  padDiskRatio->SetLeftMargin(0.095);
  padDiskRatio->SetRightMargin(0.01);
  padDiskRatio->SetFillStyle(4000);
  padDiskRatio->SetFrameFillStyle(4000);
  padDiskRatio->Draw();
  canvas->cd();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN ON-DISK GRAPH                                              //
  //--------------------------------------------------------------------------//

  padDiskSize->cd();
  gPad->SetGridy();

  int nBars = 12;

  TH1F *helperDiskSize = new TH1F("", "", nBars, 0, nBars);
  helperDiskSize->GetXaxis()->SetNdivisions(2);
  helperDiskSize->GetXaxis()->SetLabelSize(0);
  helperDiskSize->GetXaxis()->SetTickSize(0);
  if (maxInputSize > 1024)
    helperDiskSize->GetYaxis()->SetTitle("Average event size [kB]");
  else
    helperDiskSize->GetYaxis()->SetTitle("Average event size [B]");
  helperDiskSize->GetYaxis()->CenterTitle();
  helperDiskSize->GetYaxis()->SetNdivisions(6);
  helperDiskSize->GetYaxis()->SetTickSize(0.01);
  helperDiskSize->GetYaxis()->SetLabelSize(0.075);
  helperDiskSize->GetYaxis()->SetTitleSize(0.08);
  helperDiskSize->GetYaxis()->SetTitleOffset(0.55);
  helperDiskSize->SetMinimum(0);
  helperDiskSize->SetMaximum(maxSize * 1.1);
  helperDiskSize->Draw();

  for (const auto &[compression, formats] : sizeGraphMap) {
    for (const auto &[format, graph] : formats) {
      graph->SetLineColor(12);
      graph->SetMarkerColor(12);
      graph->SetFillColor(colors.at(format));
      graph->SetLineWidth(2);
      graph->Draw("B1");
      graph->Draw("P");

      // double x, y;
      // graph->GetPoint(0, x, y);

      // std::ostringstream val;
      // val.precision(1);
      // val << std::fixed << y;
      // if (maxInputSize > 1024)
      //   val << " kB";
      // else
      //   val << " B";

      // TLatex tval;
      // tval.SetTextSize(0.05);
      // tval.SetTextColor(kWhite);
      // tval.SetTextAlign(23);
      // tval.DrawLatex(x, maxSize * 0.07, val.str().c_str());
    }
  }

  for (unsigned i = 3; i < nBars; i += 3) {
    TLine *line = new TLine(i, 0, i, maxSize * 1.1);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  TLegend *leg = new TLegend(0.785, 0.745, 0.985, 0.955);
  leg->AddEntry(sizeGraphMap[505]["ttree"], "TTree", "F");
  leg->AddEntry(sizeGraphMap[505]["rntuple"], "RNTuple", "F");
  leg->SetBorderSize(1);
  // leg->SetTextSize(0.035);
  leg->Draw();

  //--------------------------------------------------------------------------//
  // DRAW THE RATIO ON-DISK GRAPH                                            //
  //--------------------------------------------------------------------------//

  padDiskRatio->cd();
  gPad->SetGridy();

  nBars = 8;

  TH1F *helperDiskRatio = new TH1F("", "", nBars, 1, nBars + 1);
  helperDiskRatio->SetMinimum(0);
  helperDiskRatio->SetMaximum(std::max(maxRatio * 1.1, 1.1));
  helperDiskRatio->GetXaxis()->SetTickSize(0);
  helperDiskRatio->GetXaxis()->SetLabelOffset(0.1);
  helperDiskRatio->GetYaxis()->SetTitle("RNTuple / TTree");
  helperDiskRatio->GetYaxis()->CenterTitle();
  helperDiskRatio->GetYaxis()->SetTickSize(0.005);
  helperDiskRatio->GetYaxis()->SetNdivisions(5);
  helperDiskRatio->GetYaxis()->SetLabelSize(0.125);
  helperDiskRatio->GetYaxis()->SetTitleSize(0.12);
  helperDiskRatio->GetYaxis()->SetTitleOffset(0.35);

  float labelSize = 0.125;

  for (int i = 0; i <= helperDiskRatio->GetXaxis()->GetNlabels(); i++) {
    if (i == 2) {
      // helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "no compression");
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "lz4");
    } else if (i == 4) {
      // helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "lz4");
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "zstd");
    } else if (i == 6) {
      // helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "zstd");
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "lzma (lvl 1)");
    } else if (i == 8) {
      // helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "lz4");
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "lzma (lvl 7)");
    } else if (i == 10) {
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, labelSize, -1, -1, -1, "lzma (lvl 7)");
    } else {
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  helperDiskRatio->Draw();

  for (const auto &[compression, graph] : ratioGraphMap) {
    graph->SetLineColor(12);
    graph->SetMarkerColor(12);
    graph->SetFillColor(colors.at("ratio"));
    graph->SetFillStyle(1001);
    graph->SetLineWidth(2);
    graph->Draw("B1");
    graph->Draw("P");

    double x, y;
    graph->GetPoint(1, x, y);
    std::ostringstream val;
    val.precision(1);
    val << std::fixed << y * 100;
    val << "%";

    TLatex tval;
    tval.SetTextSize(0.125);
    tval.SetTextColor(kWhite);
    tval.SetTextAlign(23);
    tval.DrawLatex(x, maxRatio * 0.25, val.str().c_str());
  }

  TLine *lineOneDisk = new TLine(1, 1, nBars + 1, 1);
  lineOneDisk->SetLineColor(kBlack);
  lineOneDisk->SetLineStyle(1);
  lineOneDisk->SetLineWidth(1);
  lineOneDisk->Draw();

  for (unsigned i = 2; i < 12; i += 2) {
    TLine *line = new TLine(i + 1, 0, i + 1, std::max(maxRatio * 1.1, 1.1));
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  //--------------------------------------------------------------------------//
  // SAVE THE PLOT                                                            //
  //--------------------------------------------------------------------------//

  if (save) {
    TFile *output = TFile::Open(Form("figures/size_%s.root", physFileType.c_str()), "RECREATE");
    output->cd();
    canvas->Write();
    output->Close();
    canvas->Print(Form("figures/size_%s.pdf", physFileType.c_str()));
    canvas->Print(Form("figures/size_%s.png", physFileType.c_str()));
  }
}

void plot_size() {
  SetStyle();

  makePlot("data", true);
  makePlot("mc", true);
}
