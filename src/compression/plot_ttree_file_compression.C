void plot_ttree_file_compression(
    TString resultsPath = "./data/results/results_file_compression~ttree.txt") {

  std::ifstream dataFile(resultsPath.Data());

  vector<TString> compressionSettings;
  vector<float> treeSizes;

  TString alg;
  float size;

  auto canvas = new TCanvas();
  canvas->SetGrid();
  canvas->SetTopMargin(0.1);
  canvas->SetBottomMargin(0.1);
  canvas->SetLeftMargin(0.1);
  canvas->SetRightMargin(0.1);
  canvas->cd();

  TGraph *graph = new TGraph();

  int step = 0;
  float maxSize = 0.;
  cout << "Setting\tFile size"
       << "\n---------\t---------" << endl;
  while (dataFile >> alg >> size) {
    float gbSize = size / 1024 / 1024;
    cout << alg << "\t\t" << gbSize << endl;
    compressionSettings.push_back(alg);
    treeSizes.push_back(gbSize);
    graph->SetPoint(step, step + 0.5, gbSize);

    maxSize = std::max(maxSize, gbSize);
    ++step;
  }

  TH1F *helper = new TH1F("", "", compressionSettings.size(), 0,
                          compressionSettings.size());
  graph->SetHistogram(helper);

  for (int i = 0; i < compressionSettings.size(); ++i) {
    helper->GetXaxis()->SetBinLabel(i + 1, compressionSettings[i]);
  }
  helper->GetXaxis()->SetTitle("Compression setting");
  helper->GetYaxis()->SetTitle("CollectionTree size [GB]");

  helper->SetStats(kFALSE);

  graph->Sort();
  graph->SetFillColor(kBlue);
  graph->SetMinimum(0);
  graph->SetMaximum(maxSize * 1.05);

  graph->Draw("AB");
}
