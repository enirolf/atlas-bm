#include "util.C"

int plot_ttree_type_compression(
    TString resultsBasePath =
        "./results/compression/results_type_compression.txt") {

  std::map<CompressionSetting, std::map<ClassName, TStatistic>>
      settingTypeSizeMap;

  std::ifstream resultsFile;

  ClassName className;
  double branchSize;

  for (const auto &setting : compressionSettings) {
    TString resultsPath = Form("%s~ttree-%d", resultsBasePath.Data(), setting);
    resultsFile.open(resultsPath);

    if (!resultsFile.is_open()) {
      cout << "Failed to open " << resultsPath << endl;
      return 1;
    }

    std::map<ClassName, TStatistic> typeSizeMap = {};

    while (resultsFile >> className >> branchSize) {
      if (typeSizeMap.find(className) == typeSizeMap.end()) {
        typeSizeMap[className] =
            TStatistic(Form("%s~%d", className.Data(), setting));
      }

      typeSizeMap[className].Fill(branchSize);
    }

    settingTypeSizeMap[setting] = typeSizeMap;
    resultsFile.close();
  }

  auto canvas = new TCanvas();
  canvas->SetGrid();
  canvas->cd();

  TMultiGraph *graphs = new TMultiGraph();

  double maxSize = 0.;

  for (int i = 0; i < size(compressionSettings); ++i) {
    TGraph *graph = new TGraph();
    int setting = compressionSettings[i];
    std::cout << "===============================================" << std::endl;
    std::cout << setting << std::endl;

    int step = 0;
    map<ClassName, TStatistic> typeSizeMap = settingTypeSizeMap[setting];
    for (const auto &[className, stat] : typeSizeMap) {
      std::cout << className << std::endl;
      graph->SetPoint(step, step + i, stat.GetMean());

      graph->Sort();

      graph->SetFillColor(compressionColors[i]);

      maxSize = std::max(maxSize, stat.GetMean());
      step += size(compressionSettings) + 1;
    }

    graphs->Add(graph);
    break;
  }

  graphs->Draw("ab");
  graphs->SetMinimum(0);
  return 0;
}
