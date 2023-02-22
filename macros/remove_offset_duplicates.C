#include <TTree.h>
#include <TFile.h>
#include <ROOT/TIOFeatures.hxx>

void remove_offset_duplicates(std::string tree_file_path = "data/daod_phys_benchmark_files/mc/DAOD_PHYS_MC.ttree.root~505") {
    auto file = std::unique_ptr<TFile>(TFile::Open(tree_file_path.c_str()));
    auto tree = std::unique_ptr<TTree>(file->Get<TTree>("CollectionTree"));

    std::cout << "Total bytes = " << tree->GetTotBytes()
              << "\nTotal bytes / event = " << tree->GetTotBytes() / (float) tree->GetEntries()
              << "\nZipped bytes = " << tree->GetZipBytes()
              << "\nZipped bytes / event = " << tree->GetZipBytes() / (float) tree->GetEntries()
              << std::endl;

    ROOT::TIOFeatures features;
    features.Set(ROOT::Experimental::EIOFeatures::kGenerateOffsetMap);
    tree->SetIOFeatures(features);

    auto new_file = std::unique_ptr<TFile>(TFile::Open("data/DAOD_PHYS_MC.ttree.root~OFFSETMAP", "RECREATE"));
    auto new_tree = tree->CloneTree();
    new_tree->Write();

    std::cout << "Total bytes = " << new_tree->GetTotBytes()
              << "\nTotal bytes / event = " << new_tree->GetTotBytes() / (float) new_tree->GetEntries()
              << "\nZipped bytes = " << new_tree->GetZipBytes()
              << "\nZipped bytes / event = " << new_tree->GetZipBytes() / (float) new_tree->GetEntries()
              << std::endl;
}
