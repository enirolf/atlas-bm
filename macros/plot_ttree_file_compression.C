#include "compression_util.C"

int plot_ttree_file_compression(
    TString resultsPath =
        "./results/compression/results_file_compression.txt~ttree") {

  std::ifstream resultsFile(resultsPath.Data());

  vector<float> treeSizes;

  CompressionSetting setting;
  float size;

  auto canvas = new TCanvas();
  canvas->SetGrid();

  TGraph *graph = new TGraph();

  int step = 0;
  float maxSize = 0.;
  cout << "Setting\tFile size"
       << "\n---------\t---------" << endl;
  while (resultsFile >> setting >> size) {
    float gbSize = size / (1024 * 1024);
    cout << setting << "\t\t" << gbSize << endl;
    treeSizes.push_back(gbSize);
    graph->SetPoint(step, step + 0.5, gbSize);

    maxSize = std::max(maxSize, gbSize);
    ++step;
  }

  resultsFile.close();

  TH1F *helper = new TH1F("", "", std::size(compressionSettings), 0,
                          std::size(compressionSettings));
  graph->SetHistogram(helper);

  for (int i = 0; i < std::size(compressionSettings); ++i) {
    helper->GetXaxis()->SetBinLabel(i + 1, Form("%d", compressionSettings[i]));
  }
  helper->GetXaxis()->SetTitle("Compression setting");
  helper->GetYaxis()->SetTitle("CollectionTree size [GB]");

  helper->SetStats(kFALSE);

  graph->Sort();
  graph->SetFillColor(kBlue);
  graph->SetMinimum(0);
  graph->SetMaximum(maxSize * 1.05);

  graph->Draw("AB");

  return 0;
}
