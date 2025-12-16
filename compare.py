#!/usr/bin/env python3
import sys
import re

def normalize_line(line: str) -> str:
    line = line.strip()
    if not line.startswith("atom:"):
        return line

    parts = line.split("neighbors:")
    if len(parts) != 2:
        return line

    atom_part = parts[0].rstrip(", ")
    neighbors_part = parts[1]

    tokens = [t.strip() for t in neighbors_part.split(",") if t.strip()]
    if not tokens:
        return atom_part

    neighbors_count = tokens[0]

    neighbors = []
    cur = []
    for t in tokens[1:]:
        if t.startswith("id:") and cur:
            neighbors.append(",".join(cur))
            cur = []
        cur.append(t)
    if cur:
        neighbors.append(",".join(cur))

    def get_id(n):
        m = re.search(r"id:(\d+)", n)
        return int(m.group(1)) if m else -1

    neighbors.sort(key=get_id)

    result = f"{atom_part},neighbors:{neighbors_count}"
    if neighbors:
        result += ", " + ", ".join(neighbors)

    return result


def main():
    if len(sys.argv) != 3:
        print("Использование: python diff.py <file1> <file2>")
        sys.exit(1)

    file1, file2 = sys.argv[1], sys.argv[2]

    with open(file1, encoding="utf-8") as f1, open(file2, encoding="utf-8") as f2:
        lines1 = f1.readlines()
        lines2 = f2.readlines()

    max_len = max(len(lines1), len(lines2))
    differences_found = False

    for i in range(max_len):
        raw1 = lines1[i].rstrip("\n") if i < len(lines1) else "<NO LINE>"
        raw2 = lines2[i].rstrip("\n") if i < len(lines2) else "<NO LINE>"

        norm1 = normalize_line(raw1)
        norm2 = normalize_line(raw2)

        if norm1 != norm2:
            differences_found = True
            print(f"Строка {i + 1}:")
            print(f"  {file1}: {raw1}")
            print(f"  {file2}: {raw2}")
            print()

    if not differences_found:
        print("Файлы совпадают")


if __name__ == "__main__":
    main()
