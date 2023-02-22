void rntuple_page_options(const std::string rntuple_path = "data/daod_phys_benchmark_files/data/DAOD_PHYS_DATA.rntuple.root~505") {
    auto reader = ROOT::Experimental::RNTupleReader::Open("CollectionNTuple", rntuple_path);
    auto model = reader.GetModel();

    // auto writeOptions = ROOT::Experimental::WriteOptions();
    // writeOptions.SetMaxUnzippedClusterSize(100);
    // auto writer = ROOT::Experimental::RNTupleWriter::Recreate()

}
