#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include <TTree.h>
#include <TFile.h>

#include <ROOT/RDataFrame.hxx>
#include <ROOT/RVec.hxx>

const std::string kPhysLiteFileGlob = "data/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYSLITE.e6337_s3681_r13145_r13146_p5267/*";
const float kGeV = 1000.;

void analysis() {
    ROOT::RDataFrame df("CollectionTree", kPhysLiteFileGlob);

    std::cout << "Filtering from " << std::endl;

    // auto df_goodPhotons = df.Define("goodPhotons",
    //                               [] (const ROOT::RVec<char> &isTightID,
    //                                   const ROOT::RVec<float> &photonPt,
    //                                   const ROOT::RVec<float> &photonEta)
    //                                  {
    //                                     return isTightID && (photonPt > 25000) && (abs(photonEta) < 2.37) && ((abs(photonEta) < 1.37) || (abs(photonEta) > 1.52));
    //                                  },
    //                               {"AnalysisPhotonsAuxDyn.DFCommonPhotonsIsEMTight",
    //                                "AnalysisPhotonsAuxDyn.pt",
    //                                "AnalysisPhotonsAuxDyn.eta"
    //                               })
    //                         .Filter([](const ROOT::RVec<int> &goodPhotons) { return ROOT::VecOps::Sum(goodPhotons) == 2; }, {"goodPhotons"}, "Photon filter");

    auto df_dielecNeutral = df.Filter([] (const ROOT::RVec<float> &elecCharge)
                                         {
                                            return elecCharge.size() == 2 && ROOT::VecOps::Sum(elecCharge) == 0;
                                         },
                                      {"AnalysisElectronsAuxDyn.charge"},
                                      "Exactly 2 electrons with opposite charge");

    auto df_dielecM = df_dielecNeutral.Define("dielecMass",
                                              [] (const ROOT::RVec<float> &elecMass)
                                                 {
                                                    return ROOT::VecOps::Sum(elecMass);
                                                 },
                                              {"AnalysisElectronsAuxDyn.m"}
                                             );

    auto df_dielecPt = df_dielecM.Define("dielecPt",
                                         [] (const ROOT::RVec<float> &elecPt)
                                            {
                                                return ROOT::VecOps::Sum(elecPt);
                                            },
                                         {"AnalysisElectronsAuxDyn.pt"}
                                        );

    auto df_el1Pt = df_dielecPt.Define("el1Pt",
                                       [] (const ROOT::RVec<float> &elecPt)
                                          {
                                            return elecPt[0];
                                          },
                                       {"AnalysisElectronsAuxDyn.pt"}
                                      );

    auto df_el2Pt = df_el1Pt.Define("el2Pt",
                                    [] (const ROOT::RVec<float> &elecPt)
                                       {
                                          return elecPt[1];
                                       },
                                    {"AnalysisElectronsAuxDyn.pt"}
                                   );

    std::cout << "Filtering done!" << std::endl;

    auto df_report = df_el2Pt.Report();
    df_report->Print();

    auto df_hist = df_dielecM.Histo1D<float>({"dielec_mass", "$e\\bar{e}$ mass [GeV]", 50, 0., 120.}, "dielecMass");
    df_hist->DrawCopy();
    //auto myHist2 = myDf.Histo1D<float>({"histName", "histTitle", 64u, 0., 128.}, "myColumn");
}
