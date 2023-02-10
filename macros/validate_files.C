#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTuple.hxx>

const std::string treeBasePath = "data/mc20_DAOD_PHYS.ttree.root";
const std::string ntupleBasePath = "data/mc20_DAOD_PHYS.rntuple.root";
const int compressionSettings[] = {0, 207, 404, 505};

struct EntryStats
{
    EntryStats(std::string _file, std::string _col, double _min, double _max, double _mean, double _stdev)
        : file(_file), col(_col), min(_min), max(_max), mean(_mean), stdev(_stdev) {};
    std::string file, col;
    double min, max, mean, stdev;
};

std::vector<EntryStats> ttree_branch_stats(const std::string branchName = "MuonsAuxDyn.eta", bool makeHisto = false) {
    std::vector<EntryStats> stats;

    for (int i = 0; i < sizeof(compressionSettings) / sizeof(int); ++i) {
        std::string treePath = treeBasePath + "~" + std::to_string(compressionSettings[i]);
        ROOT::RDataFrame rdf("CollectionTree", treePath);

        if (makeHisto) {
            auto hist = rdf.Histo1D({Form("hist_%d", compressionSettings[i]), "TTree", 64, -4., 4.}, branchName);
            hist->SetLineWidth(2);
            hist->SetLineColor(i + 2);
            hist->SetLineStyle(i + 1);
            hist->DrawClone("SAME");
        }

        stats.emplace_back(EntryStats(treePath, branchName, *rdf.Min(branchName), *rdf.Max(branchName), *rdf.Mean(branchName), *rdf.StdDev(branchName)));
    }

    return stats;
}

std::vector<EntryStats> rntuple_field_stats(const std::string fieldName = "MuonsAuxDyn:eta", bool makeHisto = false) {
    std::vector<EntryStats> stats;

    for (int i = 0; i < sizeof(compressionSettings) / sizeof(int); ++i) {
        std::string ntuplePath = ntupleBasePath + "~" + std::to_string(compressionSettings[i]);
        ROOT::RDataFrame rdf = ROOT::RDF::Experimental::FromRNTuple("CollectionNTuple", ntuplePath);

        if (makeHisto) {
            auto hist = rdf.Histo1D({Form("hist_%d", compressionSettings[i]), "RNTuple", 64, -4., 4.}, fieldName);
            hist->SetLineWidth(2);
            hist->SetLineColor(i + 2);
            hist->SetLineStyle(i + 1);
            hist->DrawClone("SAME");
        }

        stats.emplace_back(EntryStats(ntuplePath, fieldName, *rdf.Min(fieldName), *rdf.Max(fieldName), *rdf.Mean(fieldName), *rdf.StdDev(fieldName)));
    }

    return stats;
}

void compare_stats(const std::vector<EntryStats> allStats) {
    std::unique_ptr<EntryStats> baseStats;

    for (const auto currStats : allStats) {
        if (!baseStats) {
            baseStats = std::make_unique<EntryStats>(currStats);
        }

        if (baseStats->max != currStats.max) {
            std::cout << "max values for field/branch" << baseStats->col
                      << " do not match between " << baseStats->file
                      << " and " << currStats.file << std::endl;
            return;
        } else if (baseStats->min != currStats.min) {
            std::cout << "min values for field/branch" << baseStats->col
                      << " do not match between " << baseStats->file
                      << " and " << currStats.file << std::endl;
            return;
        } else if (baseStats->mean != currStats.mean) {
            std::cout << "mean values for field/branch" << baseStats->col
                      << " do not match between " << baseStats->file
                      << " and " << currStats.file << std::endl;
            return;
        } else if (baseStats->stdev != currStats.stdev) {
            std::cout << "stdev values for field/branch" << baseStats->col
                      << " do not match between " << baseStats->file
                      << " and " << currStats.file << std::endl;
            return;
        }
    }

    std::cout << "all stats for field/branch " << baseStats->col << " match!" << std::endl;
}

bool check_branch_field_correspondence() {
    bool correspondence = true;

    // Assume the same outcome holds for other compression settings.
    std::string treePath = treeBasePath + "~0";
    auto treeFile = TFile::Open(treePath.c_str());
    auto tree = treeFile->Get<TTree>("CollectionTree");

    std::string ntuplePath = ntupleBasePath + "~0";
    auto ntupleReader = ROOT::Experimental::RNTupleReader::Open("CollectionNTuple", ntuplePath);
    auto ntupleDescriptor = ntupleReader->GetDescriptor();


    for (const auto branch : TRangeDynCast<TBranch>(tree->GetListOfBranches())) {
        if (!branch) continue;
        std::string fieldName = branch->GetName();

        fieldName.erase(fieldName.find_last_not_of(".") + 1, std::string::npos);
        std::replace(fieldName.begin(), fieldName.end(), '.', ':');


        if (ntupleDescriptor->FindFieldId(fieldName) == -1) {
            std::cout << "unable to find field " << fieldName << " corresponding to branch " << branch->GetName() << std::endl;
            correspondence = false;
        }
    }

    for (const auto& field : ntupleDescriptor->GetTopLevelFields()) {
        std::string branchName = field.GetFieldName();

        std::replace(branchName.begin(), branchName.end(), ':', '.');

        if (!(tree->FindBranch(branchName.c_str()) || tree->FindBranch((branchName + ".").c_str()))) {
            std::cout << "unable to find branch " << branchName << " corresponding to field " << field.GetFieldName() << std::endl;
            correspondence = false;
        }
    }

    return correspondence;
}


void validate_files() {
    gInterpreter->ProcessLine("#include \"xAODMissingET/versions/MissingETCompositionBase.h\"");

    check_branch_field_correspondence();

    std::vector<std::pair<std::string, std::string>> validationCols = {
        {"MuonsAuxDyn.eta", "MuonsAuxDyn:eta"},
        {"DiTauJetsAuxDyn.phi", "DiTauJetsAuxDyn:phi"},
        {"egammaClustersAuxDyn.calM", "egammaClustersAuxDyn:calM"}
    };

    for (const auto col : validationCols) {
        auto canvas = new TCanvas(col.first.c_str(), col.first.c_str(), 1200, 600);
        canvas->Divide(2, 1);
        canvas->cd(1);
        auto allStats = ttree_branch_stats(col.first, true);

        canvas->cd(2);
        for (const auto stats : rntuple_field_stats(col.second, true)) {
            allStats.emplace_back(stats);
        }

        compare_stats(allStats);
    }
}

// std::cout << "Min  = " << rdf.Min(fieldName).GetValue()
//           << "\nMax  = " << rdf.Max(fieldName).GetValue()
//           << "\nMean = " << rdf.Mean(fieldName).GetValue()
//           << std::endl;



// return hist;
// legend->AddEntry(Form("hist_%d", compressionSettings[i]), Form("%d", compressionSettings[i]), "l");
