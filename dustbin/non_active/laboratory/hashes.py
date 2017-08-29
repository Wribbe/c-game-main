from collections import Counter
import sys
hash_lines = open(sys.argv[1]).readlines()
hash_lines = hash_lines[:-4]

hashes = []
for line in hash_lines:
    hashes.append(int(line.split("hash: ")[-1].strip()))
counter = Counter(hashes)
collisions = Counter()
for key, value in counter.items():
    if value > 1:
        collisions[value] += 1
print(collisions)
