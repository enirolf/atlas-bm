from typing import List, Tuple
from argparse import ArgumentParser

import os
import re
import statistics


def get_ttree_uncompressed_read_rates(metricsLines: List[str]) -> List[str]:
    read_rates = []
    for ln in metricsLines:
        m = re.match(r"ReadRT\s*=\s*(?P<read_rate>.*) MBytes/s", ln)
        if m:
            read_rates.append(m.group("read_rate"))
    return read_rates


def get_rntuple_uncompressed_read_rates(metricsLines: List[str]) -> List[str]:
    read_rates = []
    for ln in metricsLines:
        m = re.match(
            r"RDF\.RPageSourceFile\.bwRead\|MB/s\|bandwidth compressed bytes "
            r"read per second\|(?P<read_rate>.*)",
            ln,
        )
        if m:
            read_rates.append(m.group("read_rate"))
    return read_rates


def get_rdf_wall_times(metricsLines: List[str]) -> List[str]:
    wall_times = []
    for ln in metricsLines:
        m = re.match(
            r"Info in <\[ROOT\.RDF\] Info (.*) in void "
            r"ROOT::Detail::RDF::RLoopManager::Run\(bool\)>: Finished event "
            r"loop number . \([\.0-9]*s CPU, (?P<wall_time>[\.0-9]*)s elapsed\)\.",
            ln,
        )
        if m:
            wall_times.append(float(m.group("wall_time")))

    n = int(len(wall_times) / 10)
    return [str(sum(wall_times[i : i + n])) for i in range(0, len(wall_times), n)]


def extract_clock_times(metricsLines: List[str]) -> List[Tuple[str, str, str]]:
    clock_times = []
    for ln in metricsLines:
        if ln.startswith("180000") or ln.startswith("209062"):
            clock_times.append(tuple(ln.split()))
    return clock_times


def write_stats(stats: List[Tuple[float, ...]], path: str) -> None:
    with open(path, "w") as f:
        f.writelines("\t".join(line) + "\n" for line in stats)


if __name__ == "__main__":
    arg_parser = ArgumentParser(
        prog="extract_metrics.py",
        description="Extract relevant metrics from bm_readspeed output files",
    )
    arg_parser.add_argument("results_dir")
    args = arg_parser.parse_args()

    for root, dirs, files in os.walk(args.results_dir):
        for file in files:
            if not file.endswith(".txt"):
                continue

            path = os.path.join(root, file)

            with open(path, "r") as f:
                lines = f.readlines()
                wall_times = get_rdf_wall_times(lines)

                if "ttree" in root:
                    read_rates = get_ttree_uncompressed_read_rates(lines)
                else:
                    read_rates = get_rntuple_uncompressed_read_rates(lines)

            write_stats(list(zip(wall_times, read_rates)), path[:-4] + ".data")
