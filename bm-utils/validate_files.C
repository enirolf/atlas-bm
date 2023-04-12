#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <regex>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTuple.hxx>

const std::string treePath = "data/DAOD_PHYS.art.pool.root";
const std::string ntuplePath = "data/DAOD_PHYS.art.pool.rntuple";
const int compressionSettings[] = {505};

struct EntryStats
{
    EntryStats(std::string _file, std::string _col, double _min, double _max, double _mean, double _stdev)
        : file(_file), col(_col), min(_min), max(_max), mean(_mean), stdev(_stdev) {};
    std::string file, col;
    double min, max, mean, stdev;
};

bool replace_substr(std::string& str, std::string_view from, std::string_view to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

std::string get_field_name(std::string_view branchName) {
    std::string fieldName = std::string(branchName);
    replace_substr(fieldName, "Dyn", ":");
    std::replace(fieldName.begin(), fieldName.end(), '.', ':');

    return fieldName;
}

std::string get_branch_name(std::string_view fieldName) {
    std::string branchName = std::string(fieldName);
    branchName.erase(branchName.find_last_not_of(".") + 1, std::string::npos);
    replace_substr(branchName, "Aux::", "AuxDyn.");
    replace_substr(branchName, "Aux:", "Aux.");
    std::replace(branchName.begin(), branchName.end(), ':', '.');

    return branchName;
}

std::vector<EntryStats> ttree_branch_stats(const std::string branchName = "PhotonsAuxDyn.eta", bool makeHisto = false) {
    std::vector<EntryStats> stats;

    ROOT::RDataFrame rdf("CollectionTree", treePath);

    if (makeHisto) {
        auto hist = rdf.Histo1D({"hist_ttree", "TTree", 64, -4., 4.}, branchName);
        hist->SetLineWidth(2);
        hist->SetLineColor(2);
        hist->SetLineStyle(1);
        hist->DrawClone("SAME");
    }

    stats.emplace_back(EntryStats(treePath, branchName, *rdf.Min(branchName), *rdf.Max(branchName), *rdf.Mean(branchName), *rdf.StdDev(branchName)));


    return stats;
}

std::vector<EntryStats> rntuple_field_stats(const std::string fieldName = "PhotonsAux::eta", bool makeHisto = false) {
    std::vector<EntryStats> stats;

    ROOT::RDataFrame rdf = ROOT::RDF::Experimental::FromRNTuple("RNT:CollectionTree", ntuplePath);

    if (makeHisto) {
        auto hist = rdf.Histo1D({"hist_rntuple", "RNTuple", 64, -4., 4.}, fieldName);
        hist->SetLineWidth(2);
        hist->SetLineColor(2);
        hist->SetLineStyle(1);
        hist->DrawClone("SAME");
    }

    stats.emplace_back(EntryStats(ntuplePath, fieldName, *rdf.Min(fieldName), *rdf.Max(fieldName), *rdf.Mean(fieldName), *rdf.StdDev(fieldName)));

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
    auto treeFile = TFile::Open(treePath.c_str());
    auto tree = treeFile->Get<TTree>("CollectionTree");

    auto ntupleReader = ROOT::Experimental::RNTupleReader::Open("RNT:CollectionTree", ntuplePath);
    auto ntupleDescriptor = ntupleReader->GetDescriptor();


    for (const auto branch : TRangeDynCast<TBranch>(tree->GetListOfBranches())) {
        if (!branch) continue;
        std::string fieldName = get_field_name(branch->GetName());

        if (ntupleDescriptor->FindFieldId(fieldName) == -1) {
            std::cout << "unable to find field " << fieldName << " corresponding to branch " << branch->GetName() << std::endl;
            correspondence = false;
        }
    }

    for (const auto& field : ntupleDescriptor->GetTopLevelFields()) {
        std::string branchName = get_branch_name(field.GetFieldName());

        if (!(tree->FindBranch(branchName.c_str()) || tree->FindBranch((branchName + ".").c_str()))) {
            std::cout << "unable to find branch " << branchName << " corresponding to field " << field.GetFieldName() << std::endl;
            correspondence = false;
        }
    }

    return correspondence;
}


void validate_files() {
    gInterpreter->ProcessLine("#include <xAODMissingET/versions/MissingETCompositionBase.h>");
    gInterpreter->ProcessLine("#include <xAODMissingET/versions/MissingETBase.h>");
    gInterpreter->ProcessLine("#include <CxxUtils/sgkey_t.h>");

    check_branch_field_correspondence();

    // std::vector<std::pair<std::string, std::string>> validationCols = {
    //     {"PhotonsAuxDyn.eta", "PhotonsAux::eta"},
    //     {"DiTauJetsAuxDyn.phi", "DiTauJetsAux::phi"},
    //     {"egammaClustersAuxDyn.calM", "egammaClustersAux::calM"}
    // };

    // for (const auto col : validationCols) {
    //     auto canvas = new TCanvas(col.first.c_str(), col.first.c_str(), 1200, 600);
    //     canvas->Divide(2, 1);
    //     canvas->cd(1);
    //     auto allStats = ttree_branch_stats(col.first, true);

    //     canvas->cd(2);
    //     for (const auto stats : rntuple_field_stats(col.second, true)) {
    //         allStats.emplace_back(stats);
    //     }

    //     compare_stats(allStats);
    // }
}

// std::cout << "Min  = " << rdf.Min(fieldName).GetValue()
//           << "\nMax  = " << rdf.Max(fieldName).GetValue()
//           << "\nMean = " << rdf.Mean(fieldName).GetValue()
//           << std::endl;



// return hist;
// legend->AddEntry(Form("hist_%d", compressionSettings[i]), Form("%d", compressionSettings[i]), "l");
