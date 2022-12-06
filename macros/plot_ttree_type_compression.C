#include "compression_util.C"

// float ContainerSize(const std::vector<ContainerProperties> *container);
// float CategorySize(const CategoryProperties *category);

float ContainerSize(const std::vector<ContainerProperties> *container) {
  float totSize = 0.;

  for (const auto &conProp : *container) {
    totSize += conProp.diskSize;
  }

  return totSize;
}

float CategorySize(const CategoryProperties *category) {
  float totSize = 0.;

  for (const auto &container : category->containerProperties) {
    totSize += ContainerSize(&container.second);
  }

  return totSize;
}

int plot_ttree_type_compression(
    std::string resultsBasePath =
        "./results/compression/results_type_compression.txt") {

  // std::map<CompressionSetting, std::map<std::string, TStatistic>>
  //     settingTypeSizeMap;

  std::map<CompressionSetting, CategorySizeMap> sizeMap;

  std::ifstream resultsFile;

  std::string className;
  std::string branchName;
  double branchSize;

  // CategoryGraphMap catGraphMap;
  // InitCategoryGraphMap(&catGraphMap);
  auto canvas = new TCanvas();
  canvas->SetGrid();
  canvas->cd();

  TMultiGraph *graphs = new TMultiGraph();

  /* Read and process the results for every compression setting. */
  for (int i = 0; i < size(compressionSettings); ++i) {
    CompressionSetting setting = compressionSettings[i];
    TString resultsPath = Form("%s~ttree-%d", resultsBasePath.c_str(), setting);
    resultsFile.open(resultsPath);

    if (!resultsFile.is_open()) {
      cout << "Failed to open " << resultsPath << endl;
      return 1;
    }

    CategorySizeMap catMap = {};
    InitCategorySizeMap(&catMap, setting, i);

    /* Build the map containing all categories and their respective containers.
     */
    while (resultsFile >> className >> branchName >> branchSize) {
      ContainerName containerName = GetContainerName(branchName);
      CategoryName categoryName = GetCategory(containerName);

      std::map<ContainerName, std::vector<ContainerProperties>>
          &containerProps = catMap.at(categoryName)->containerProperties;

      if (containerProps.find(containerName) == containerProps.end()) {
        containerProps[containerName] = {
            ContainerProperties(branchName, className, branchSize)};
      } else {
        containerProps[containerName].push_back(
            ContainerProperties(branchName, className, branchSize));
      }
    }

    resultsFile.close();

    /* For now, only plot per entire category. */
    TGraph *graph = new TGraph();

    int x = 0;
    for (const auto &[catName, catProps] : catMap) {
      float diskSize = CategorySize(catProps);

      if (diskSize == 0.) {
        continue;
      }

      int step = catProps->stepSize;
      graph->SetPoint(step, x + step, diskSize);
      std::cout << catName << ": " << diskSize << " (" << step << ", " << x
                << ")" << std::endl;

      x += size(compressionSettings);
    }

    graph->Sort();
    graph->SetFillColor(compressionColors[i]);
    graphs->Add(graph);

    /* For testing, remove! */

    // break;
  }

  graphs->Print();
  graphs->Draw("ab");
  graphs->SetMinimum(0);
  return 0;
}
