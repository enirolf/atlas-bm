from typing import List

import re

TEST_RNTUPLE_METRICS = """
RNTupleReader.RPageSourceFile.nReadV||number of vector read requests|89
RNTupleReader.RPageSourceFile.nRead||number of byte ranges read|1567
RNTupleReader.RPageSourceFile.szReadPayload|B|volume read from storage (required)|245541537
RNTupleReader.RPageSourceFile.szReadOverhead|B|volume read from storage (overhead)|18232427
RNTupleReader.RPageSourceFile.szUnzip|B|volume after unzipping|444707472
RNTupleReader.RPageSourceFile.nClusterLoaded||number of partial clusters preloaded from storage|89
RNTupleReader.RPageSourceFile.nPageLoaded||number of pages loaded from storage|10192
RNTupleReader.RPageSourceFile.nPagePopulated||number of populated pages|10192
RNTupleReader.RPageSourceFile.timeWallRead|ns|wall clock time spent reading|617597866
RNTupleReader.RPageSourceFile.timeWallUnzip|ns|wall clock time spent decompressing|15587110619
RNTupleReader.RPageSourceFile.timeCpuRead|ns|CPU time spent reading|729446000
RNTupleReader.RPageSourceFile.timeCpuUnzip|ns|CPU time spent decompressing|15729541000
RNTupleReader.RPageSourceFile.bwRead|MB/s|bandwidth compressed bytes read per second|427.096625
RNTupleReader.RPageSourceFile.bwReadUnzip|MB/s|bandwidth uncompressed bytes read per second|720.059923
RNTupleReader.RPageSourceFile.bwUnzip|MB/s|decompression bandwidth of uncompressed bytes per second|28.530462
RNTupleReader.RPageSourceFile.rtReadEfficiency||ratio of payload over all bytes read|0.930879
RNTupleReader.RPageSourceFile.rtCompression||ratio of compressed bytes / uncompressed bytes|0.552142
"""

TEST_TTREE_METRICS = """
TreeCache = 23 MBytes
N leaves  = 1928
ReadTotal = 278.334 MBytes
ReadUnZip = 466.995 MBytes
ReadCalls = 3314
ReadSize  =  83.987 KBytes/read
Readahead = 256 KBytes
Readextra = 28.54 per cent
Real Time =   2.098 seconds
CPU  Time =   1.520 seconds
Disk Time =   0.664 seconds
Disk IO   = 419.274 MBytes/s
ReadUZRT  = 222.593 MBytes/s
ReadUZCP  = 307.233 MBytes/s
ReadRT    = 132.667 MBytes/s
ReadCP    = 183.114 MBytes/s
"""


def extract_ttree_uncompressed_read_rates(metricsLines: str) -> List[str]:
    read_rates = []
    for ln in metricsLines.splitlines():
        m = re.match(r"ReadUZRT\s*=\s*(?P<read_rate>.*) MBytes/s", ln)
        if m:
            read_rates.append(m.group("read_rate"))
    return read_rates


def extract_rntuple_uncompressed_read_rates(metricsLines: str) -> List[float]:
    read_rates = []
    for ln in metricsLines.splitlines():
        m = re.match(
            r"RNTupleReader\.RPageSourceFile\.bwReadUnzip\|MB/s\|bandwidth uncompressed bytes read per second\|(?P<read_rate>.*)",
            ln,
        )
        if m:
            read_rates.append(float(m.group("read_rate")))
    return read_rates


def extract_clock_times(metricsLines: str) -> List[str]:
    clock_times = []
    for ln in metricsLines.splitlines():
        if ln.startswith("180000") or ln.startswith("209062"):
            clock_times.append(ln)
    return clock_times


if __name__ == "__main__":
    print(extract_ttree_uncompressed_read_rates(TEST_TTREE_METRICS))
    print(extract_rntuple_uncompressed_read_rates(TEST_RNTUPLE_METRICS))
