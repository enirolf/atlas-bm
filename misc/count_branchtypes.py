import collections

N_BRANCHES = 1312

if __name__ == '__main__':
  with open('results/daod_phys_branch_types.txt') as f:
    branch_types = [l.strip() for l in f.readlines()]

  branch_frequencies = collections.Counter(branch_types)

  for branch, freq in branch_frequencies.most_common():
    print(f'{branch}; {freq}')
