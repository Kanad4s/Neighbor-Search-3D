#!/usr/bin/env python3
import sys

if len(sys.argv) != 3:
    print("Использование: python diff.py <file1> <file2>")
    sys.exit(1)

file1, file2 = sys.argv[1], sys.argv[2]

with open(file1, encoding="utf-8") as f1, open(file2, encoding="utf-8") as f2:
    lines1 = f1.readlines()
    lines2 = f2.readlines()

max_len = max(len(lines1), len(lines2))

for i in range(max_len):
    line1 = lines1[i].rstrip("\n") if i < len(lines1) else "<NO LINE>"
    line2 = lines2[i].rstrip("\n") if i < len(lines2) else "<NO LINE>"

    if line1 != line2:
        print(f"Строка {i + 1}:")
        print(f"  {file1}: {line1}")
        print(f"  {file2}: {line2}")
        print()
