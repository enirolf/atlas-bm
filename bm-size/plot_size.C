#include "plot_util.C"

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
  std::string compression;
  uint64_t nEvents, onDiskSize, inMemorySize;

  std::vector<std::string> formats;
  // compression -> storage format -> TGraphErrors*
  std::map<std::string, std::map<std::string, TGraphErrors *>> sizeGraphMap;
  // compression -> storage format -> size/event
  std::map<std::string, std::map<std::string, float>> dataMap;
  // compression -> TGraphErrors*
  std::map<std::string, TGraphErrors *> ratioGraphMap;

  float maxSize = 0.;
  while (file >> format >> compression >> nEvents >> onDiskSize >> inMemorySize) {
    float eventSize;

    if (dataMap.find("memory") == dataMap.end() ||
        dataMap.at("memory").find(format) == dataMap.at("memory").end()) {
      eventSize = inMemorySize / nEvents;
      dataMap["memory"][format] = eventSize;
    }

    eventSize = onDiskSize / nEvents;
    dataMap[compression][format] = eventSize;
    maxSize = std::max(maxSize, eventSize);
  }

  float maxRatio = 0.;
  for (const auto &[compression, formats] : dataMap) {
    for (const auto &[format, data] : formats) {
      float eventSize = dataMap[compression][format];

      if (maxSize < 1024)
        eventSize *= 1024.;

      int x;
      if (compression == "memory") {
        x = (format == "rntuple");
      } else {
        x = (compression == "zstd") * 2 + (format == "rntuple");
      }

      auto g = new TGraphErrors();
      g->SetPoint(0, x + 0.5, eventSize);
      g->SetPoint(1, x + 2., -1);
      g->SetPointError(0, 0, 0);
      sizeGraphMap[compression][format] = g;
    }

    float ratio = dataMap[compression]["rntuple"] / dataMap[compression]["ttree"];
    maxRatio = std::max(maxRatio, ratio);

    int x = (compression == "zstd");

    auto g = new TGraphErrors();
    g->SetPoint(0, 2 * x + 0, -1);
    g->SetPoint(1, 2 * x + 1.5, ratio);
    g->SetPoint(2, 2 * x + 3, -1);
    g->SetPointError(1, 0, 0);
    ratioGraphMap[compression] = g;
  }

  //--------------------------------------------------------------------------//
  // SETUP THE CANVAS                                                         //
  //--------------------------------------------------------------------------//

  TCanvas *canvas = new TCanvas(Form("canvas_%s", physFileType.c_str()), "canvas", 1200, 600);
  canvas->cd();

  // Title pad
  auto padTitle = new TPad("padTitle", "padTitle", 0.0, 0.9, 1.0, 0.95);
  padTitle->SetTopMargin(0.08);
  padTitle->SetBottomMargin(0.03);
  padTitle->SetLeftMargin(0.1);
  padTitle->SetRightMargin(0.005);
  padTitle->Draw();
  canvas->cd();

  // TTree and RNTuple on-disk size (no comp. + zstd)
  auto padDiskSize = new TPad("padDiskSize", "padDiskSize", 0.0, 0.39, 0.66, 0.89);
  padDiskSize->SetTopMargin(0.08);
  padDiskSize->SetBottomMargin(0.03);
  padDiskSize->SetLeftMargin(0.1);
  padDiskSize->SetRightMargin(0.005);
  padDiskSize->Draw();
  canvas->cd();

  // TTree and RNTuple in-memory size
  auto padMemSize = new TPad("padMemSize", "padMemSize", 0.66, 0.39, 1.0, 0.89);
  padMemSize->SetTopMargin(0.08);
  padMemSize->SetBottomMargin(0.03);
  padMemSize->SetLeftMargin(0.1);
  padMemSize->SetRightMargin(0.055);
  padMemSize->Draw();
  canvas->cd();

  // TTree vs RNTuple on-disk size ratio (no comp. + zstd)
  auto padDiskRatio = new TPad("padDiskRatio", "padDiskRatio", 0.0, 0.09, 0.66, 0.38);
  padDiskRatio->SetTopMargin(0.05);
  padDiskRatio->SetBottomMargin(0.26);
  padDiskRatio->SetLeftMargin(0.1);
  padDiskRatio->SetRightMargin(0.005);
  padDiskRatio->Draw();
  canvas->cd();

  // TTree vs RNTuple in-memory size ratio
  auto padMemRatio = new TPad("padMemRatio", "padMemRatio", 0.66, 0.09, 1.0, 0.38);
  padMemRatio->SetTopMargin(0.05);
  padMemRatio->SetBottomMargin(0.26);
  padMemRatio->SetLeftMargin(0.1);
  padMemRatio->SetRightMargin(0.055);
  padMemRatio->Draw();
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
      new TText(0.5, 0.5, Form("TTree vs. RNTuple storage efficiency (%s)", physFileType.c_str()));
  title->SetBBoxCenter(padTitle->GetBBoxCenter());
  title->SetTextColor(kBlack);
  title->SetTextSize(.9);
  title->SetTextAlign(23);
  title->Draw();

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN ON-DISK GRAPH                                              //
  //--------------------------------------------------------------------------//

  padDiskSize->cd();
  gPad->SetGridy();

  TH1F *helperDiskSize = new TH1F("", "", 4, 0, 4);
  helperDiskSize->GetXaxis()->SetNdivisions(2);
  helperDiskSize->GetXaxis()->SetLabelSize(0);
  helperDiskSize->GetXaxis()->SetTickSize(0);
  if (maxSize > 1024)
    helperDiskSize->GetYaxis()->SetTitle("Average event size [kB]");
  else
    helperDiskSize->GetYaxis()->SetTitle("Average event size [B]");
  helperDiskSize->GetYaxis()->CenterTitle();
  helperDiskSize->GetYaxis()->SetTickSize(0.01);
  helperDiskSize->GetYaxis()->SetLabelSize(0.055);
  helperDiskSize->GetYaxis()->SetTitleSize(0.055);
  helperDiskSize->GetYaxis()->SetTitleOffset(0.725);
  helperDiskSize->SetMinimum(0);
  helperDiskSize->SetMaximum(maxSize * 1.1);
  helperDiskSize->SetTitle("On-disk");

  helperDiskSize->Draw();

  for (const auto &[compression, formats] : sizeGraphMap) {
    if (compression == "memory")
      continue;

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
    TLine *line = new TLine(i, 0, i, maxSize * 1.1);
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  //--------------------------------------------------------------------------//
  // DRAW THE MAIN IN-MEMORY GRAPH                                            //
  //--------------------------------------------------------------------------//

  padMemSize->cd();
  gPad->SetGridy();

  TH1F *helperMemSize = new TH1F("", "", 2, 0, 2);
  helperMemSize->GetXaxis()->SetNdivisions(2);
  helperMemSize->GetXaxis()->SetLabelSize(0);
  helperMemSize->GetXaxis()->SetTickSize(0);
  helperMemSize->GetYaxis()->SetTickSize(0.01);
  helperMemSize->GetYaxis()->SetLabelSize(0.055);
  helperMemSize->GetYaxis()->SetTitleSize(0.055);
  helperMemSize->SetMinimum(0);
  helperMemSize->SetMaximum(maxSize * 1.1);
  helperMemSize->SetTitle("In-memory");

  helperMemSize->Draw();

  for (const auto &[compression, formats] : sizeGraphMap) {
    if (compression != "memory")
      continue;

    for (const auto &[format, graph] : formats) {
      graph->SetLineColor(12);
      graph->SetMarkerColor(12);
      graph->SetFillColor(colors.at(format));
      graph->SetLineWidth(2);
      graph->Draw("B1");
      graph->Draw("P");
    }
  }

  //--------------------------------------------------------------------------//
  // DRAW THE RATIO ON-DISK GRAPH                                            //
  //--------------------------------------------------------------------------//

  padDiskRatio->cd();
  gPad->SetGridy();

  TH1F *helperDiskRatio = new TH1F("", "", 4, 0.5, 4.5);
  helperDiskRatio->SetMinimum(0);
  helperDiskRatio->SetMaximum(std::max(maxRatio * 1.1, 1.1));
  helperDiskRatio->GetXaxis()->SetTickSize(0);
  helperDiskRatio->GetXaxis()->SetLabelSize(0.16);
  helperDiskRatio->GetXaxis()->SetTitleSize(0.095);
  helperDiskRatio->GetYaxis()->SetTitle("RNTuple / TTree");
  helperDiskRatio->GetYaxis()->CenterTitle();
  helperDiskRatio->GetYaxis()->SetTickSize(0.005);
  helperDiskRatio->GetYaxis()->SetNdivisions(6);
  helperDiskRatio->GetYaxis()->SetLabelSize(0.095);
  helperDiskRatio->GetYaxis()->SetTitleSize(0.095);
  helperDiskRatio->GetYaxis()->SetTitleOffset(0.45);

  for (int i = 0; i <= helperDiskRatio->GetXaxis()->GetNlabels(); i++) {
    if (i == 3) {
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, 0.095, -1, -1, -1, "no compression");
    } else if (i == 7) {
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, 0.095, -1, -1, -1, "zstd compression");
    } else {
      helperDiskRatio->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  helperDiskRatio->Draw();

  for (const auto &[compression, graph] : ratioGraphMap) {
    if (compression == "memory")
      continue;

    graph->SetLineColor(12);
    graph->SetMarkerColor(12);
    graph->SetFillColor(kPlotGreen);
    graph->SetFillStyle(styles.at(compression));
    graph->SetLineWidth(2);
    graph->Draw("B1");
    graph->Draw("P");

    double x, y, err;
    graph->GetPoint(1, x, y);
    std::ostringstream val;
    val.precision(1);
    val << std::fixed << y * 100;
    val << "%";
    TLatex tval;
    tval.SetTextSize(0.08);
    tval.SetTextAlign(23);
    tval.DrawLatex(x, y * 0.8, val.str().c_str());
  }

  TLine *lineOneDisk = new TLine(0.5, 1, 4.5, 1);
  lineOneDisk->SetLineColor(kBlack);
  lineOneDisk->SetLineStyle(1);
  lineOneDisk->SetLineWidth(1);
  lineOneDisk->Draw();

  for (unsigned i = 2; i < 4; i += 2) {
    TLine *line = new TLine(i + 0.5, 0, i + 0.5, std::max(maxRatio * 1.1, 1.1));
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  //--------------------------------------------------------------------------//
  // DRAW THE RATIO IN-MEMORY GRAPH                                           //
  //--------------------------------------------------------------------------//

  padMemRatio->cd();
  gPad->SetGridy();

  TH1F *helperMemRatio = new TH1F("", "", 2, 0.5, 2.5);
  helperMemRatio->SetMinimum(0);
  helperMemRatio->SetMaximum(std::max(maxRatio * 1.1, 1.1));
  helperMemRatio->GetXaxis()->SetTickSize(0);
  helperMemRatio->GetXaxis()->SetLabelSize(0.15);
  helperMemRatio->GetXaxis()->SetTitleSize(0.095);
  helperMemRatio->GetYaxis()->SetTickSize(0.005);
  helperMemRatio->GetYaxis()->SetNdivisions(6);
  helperMemRatio->GetYaxis()->SetLabelSize(0.095);
  helperMemRatio->GetYaxis()->SetTitleSize(0.095);

  for (int i = 0; i <= helperMemRatio->GetXaxis()->GetNlabels(); i++) {
    if (i == 5) {
      helperMemRatio->GetXaxis()->ChangeLabel(i, -1, 0.095, -1, -1, -1, "in-memory");
    } else {
      helperMemRatio->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  helperMemRatio->Draw();

  for (const auto &[compression, graph] : ratioGraphMap) {
    if (compression != "memory")
      continue;

    graph->SetLineColor(12);
    graph->SetMarkerColor(12);
    graph->SetFillColor(kPlotGreen);
    graph->SetFillStyle(styles.at(compression));
    graph->SetLineWidth(2);
    graph->Draw("B1");
    graph->Draw("P");

    double x, y, err;
    graph->GetPoint(1, x, y);
    std::ostringstream val;
    val.precision(1);
    val << std::fixed << y * 100;
    val << "%";
    TLatex tval;
    tval.SetTextSize(0.08);
    tval.SetTextAlign(23);
    tval.DrawLatex(x, y * 0.8, val.str().c_str());
  }

  TLine *lineOneMem = new TLine(0.5, 1, 2.5, 1);
  lineOneMem->SetLineColor(kBlack);
  lineOneMem->SetLineStyle(1);
  lineOneMem->SetLineWidth(1);
  lineOneMem->Draw();

  for (unsigned i = 2; i < 4; i += 2) {
    TLine *line = new TLine(i + 0.5, 0, i + 0.5, std::max(maxRatio * 1.1, 1.1));
    line->SetLineColor(kBlack);
    line->SetLineStyle(3);
    line->SetLineWidth(1);
    line->Draw();
  }

  //--------------------------------------------------------------------------//
  // DRAW THE LEGEND                                                          //
  //--------------------------------------------------------------------------//

  padLegend->cd();

  TLegend *leg = new TLegend(0.3, 0.1, 0.7, 0.9);
  leg->AddEntry(sizeGraphMap["zstd"]["ttree"], "TTree", "F");
  leg->AddEntry(sizeGraphMap["zstd"]["rntuple"], "RNTuple", "F");
  leg->AddEntry(ratioGraphMap["zstd"], "RNTuple / TTree storage ratio", "F");
  leg->SetNColumns(3);
  leg->SetColumnSeparation(.15);
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
    canvas->Print(Form("figures/size_%s.pdf", physFileType.c_str()));
    canvas->Print(Form("figures/size_%s.png", physFileType.c_str()));
  }
}

void plot_size() {
  SetStyle();

  makePlot("data");
  makePlot("mc");
}
