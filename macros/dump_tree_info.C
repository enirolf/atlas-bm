#include <TFile.h>
#include <TTree.h>

#include <memory>
#include <iostream>
#include <string>
#include <utility>
#include <algorithm>

struct BranchInfo
{
    BranchInfo (std::uint64_t t, std::uint64_t z, std::string b, std::string c) : compression_factor((float)t / (float)z), branch_name(b), class_name(c) {};

    float compression_factor;
    std::string branch_name, class_name;
};

bool sort_branch_info_desc(BranchInfo const& fst, BranchInfo const& snd) {
    return fst.compression_factor > snd.compression_factor;
}

void dump_branch_compression(const std::unique_ptr<TTree> tree, int n_branches = 10) {
    std::vector<BranchInfo> branch_info;

    for (auto br : TRangeDynCast<TBranch>(tree->GetListOfBranches())) {
        if (!br) continue;

        // std::cout << br->GetName() << "\t" << br->GetTotBytes() << "\t" << br->GetZipBytes() << std::endl;
        branch_info.emplace_back(BranchInfo(br->GetTotBytes(), br->GetZipBytes(), br->GetName(), br->GetClassName()));
    }

    std::sort(branch_info.begin(), branch_info.end(), &sort_branch_info_desc);

    for (size_t i = 0; i < n_branches; i++)
    {
        std::cout << branch_info[i].branch_name << " <" << branch_info[i].class_name << "> :\t" << branch_info[i].compression_factor << std::endl;
    }

}

void dump_tree_info(std::string file_path = "data/daod_phys_benchmark_files/data/DAOD_PHYS_DATA.ttree.root~505") {
    auto file = std::unique_ptr<TFile>(TFile::Open(file_path.c_str()));
    auto tree = std::unique_ptr<TTree>(file->Get<TTree>("CollectionTree"));

    dump_branch_compression(std::move(tree), 25);
}
