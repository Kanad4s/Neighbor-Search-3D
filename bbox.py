import sys
import math

if len(sys.argv) < 2:
    print(f"Usage: python {sys.argv[0]} file.txt")
    sys.exit(1)

filename = sys.argv[1]

xmin = ymin = zmin = math.inf
xmax = ymax = zmax = -math.inf

with open(filename, "r") as f:
    # пропускаем первые три строки
    next(f)
    next(f)
    next(f)

    # читаем координаты
    for line in f:
        line = line.strip()
        if not line:
            continue

        parts = line.split()
        if len(parts) < 3:
            continue

        # первые 3 числа — координаты
        x, y, z = map(float, parts[:3])

        xmin = min(xmin, x)
        ymin = min(ymin, y)
        zmin = min(zmin, z)

        xmax = max(xmax, x)
        ymax = max(ymax, y)
        zmax = max(zmax, z)

print("Bounding box:")
print(f"xmin = {xmin:.10f}, xmax = {xmax:.10f}, Lx = {xmax - xmin:.10f}")
print(f"ymin = {ymin:.10f}, ymax = {ymax:.10f}, Ly = {ymax - ymin:.10f}")
print(f"zmin = {zmin:.10f}, zmax = {zmax:.10f}, Lz = {zmax - zmin:.10f}")
