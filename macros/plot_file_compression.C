#include "compression_util.C"

struct ResultProps {
  ResultProps(std::string f, EColor c, int s) : format(f), color(c), step(s) {}

  std::string format;
  EColor color;
  int step;
};

int plot_file_compression(
    std::string resultsBase =
        "./results/compression/results_file_compression.txt") {

  std::ifstream resultsFile;

  ResultProps resProps[] = {
      ResultProps("TTree", kBlue, 0),
      ResultProps("TTree", kRed, 1)}; // Change to RNTuple!!

  CompressionSetting setting;
  int nEvents;
  float totSize;

  TMultiGraph *graphs = new TMultiGraph();
  float maxSize = 0.;

  for (const auto &resProp : resProps) {
    std::cout << "=====" << resProp.format << "=====" << std::endl;
    std::string resultsPath =
        Form("%s~%s", resultsBase.c_str(), resProp.format.c_str());
    resultsFile.open(resultsPath);

    if (!resultsFile.is_open()) {
      cout << "Failed to open " << resultsPath << endl;
      return 1;
    }

    gStyle->SetBarWidth(0.625);

    TGraph *graph = new TGraph();
    int n = 0;

    cout << "Setting\tTotal size\tSize/Event"
         << "\n---------\t---------\t---------" << endl;
    while (resultsFile >> setting >> nEvents >> totSize) {
      float eventSize = totSize / nEvents;
      std::cout << setting << "\t" << totSize << "\t" << eventSize << std::endl;

      float x = (n * 2 + resProp.step) / 2. + 0.5;
      graph->SetPoint(n, x, eventSize + (0.5 * resProp.step * eventSize));

      maxSize = std::max(maxSize, eventSize);
      ++n;
    }

    resultsFile.close();

    graph->Sort();
    graph->SetFillColor(resProp.color);
    graphs->Add(graph);
  }

  // TH1F *helper = new TH1F("", "", std::size(compressionSettings), 0,
  //                         std::size(compressionSettings));
  TH1F *axisHist = graphs->GetHistogram();
  // graph->SetHistogram(helper);
  // axisHist->GetXaxis()->SetNdivisions(size(compressionSettings));
  int nBins = axisHist->GetXaxis()->GetNbins();
  for (int i = 0; i < std::size(compressionSettings); ++i) {
    int bin = (nBins / size(compressionSettings)) * (i + 1) -
              ((nBins / size(compressionSettings)) / 2);
    axisHist->GetXaxis()->SetBinLabel(bin, Form("%d", compressionLabels[i]));
  }
  axisHist->GetXaxis()->SetTitle("Compression setting");
  axisHist->GetYaxis()->SetTitle("CollectionTree totSize [GB]");

  axisHist->SetStats(kFALSE);

  auto canvas = new TCanvas();
  canvas->SetGrid(2, 1);
  canvas->cd();

  graphs->Print();
  graphs->Draw("AB");
  graphs->SetMaximum(maxSize * 2);
  graphs->SetMinimum(0);

  return 0;
}
