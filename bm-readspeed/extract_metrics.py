from typing import List, Tuple

import os
import re


def extract_ttree_uncompressed_read_rates(metricsLines: List[str]) -> List[str]:
    read_rates = []
    for ln in metricsLines:
        m = re.match(r"ReadUZRT\s*=\s*(?P<read_rate>.*) MBytes/s", ln)
        if m:
            read_rates.append(m.group("read_rate"))
    return read_rates


def extract_rntuple_uncompressed_read_rates(metricsLines: List[str]) -> List[str]:
    read_rates = []
    for ln in metricsLines:
        m = re.match(
            r"RNTupleReader\.RPageSourceFile\.bwReadUnzip\|MB/s\|bandwidth uncompressed bytes read per second\|(?P<read_rate>.*)",
            ln,
        )
        if m:
            read_rates.append(m.group("read_rate"))
    return read_rates


def extract_clock_times(metricsLines: List[str]) -> List[Tuple[str, str, str]]:
    clock_times = []
    for ln in metricsLines:
        if ln.startswith("180000") or ln.startswith("209062"):
            clock_times.append(tuple(ln.split()))
    return clock_times


def write_read_rates(read_rates: List[str], path: str) -> None:
    with open(path, "w") as f:
        f.writelines(line + "\n" for line in read_rates)


def write_clock_times(clock_times: List[Tuple[str, str, str]], path: str) -> None:
    with open(path, "w") as f:
        f.writelines("\t".join(line) + "\n" for line in clock_times)


if __name__ == "__main__":
    for dir_name in ["ssd_verbose", "hdd_verbose"]:
        for root, dirs, files in os.walk("results/" + dir_name):
            for file in files:
                if not file.endswith(".txt"):
                    continue

                path = os.path.join(root, file)
                with open(path, "r") as f:
                    lines = f.readlines()
                    if "tree" in root:
                        write_read_rates(
                            extract_ttree_uncompressed_read_rates(lines),
                            path + "~read_rates",
                        )
                    else:
                        write_read_rates(
                            extract_rntuple_uncompressed_read_rates(lines),
                            path + "~read_rates",
                        )

                    write_clock_times(extract_clock_times(lines), path + "~clock_times")
