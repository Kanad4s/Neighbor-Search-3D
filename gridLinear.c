#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

const int neighborsCount = 4;
const double neighborRadius = 2.5;

typedef struct {
    int id;
    double x, y, z;
    int neighbors;
    int preNeighbors;
} Atom;

typedef struct {
    Atom* atoms;
    int atomsCount;
    int xAtomsCount, yAtomsCount, zAtomsCount;
} GridCell;

typedef struct {
    GridCell* cells;
    int xCellsCount, yCellsCount, zCellsCount;
    int atomsCount;
} Grid;

typedef struct {
    double x, y, z;
} Substract;

int isNeighbor(Atom a, Atom b);
Atom* getAtoms(int count);
Atom** findNeighbors(Grid* grid);
int read_csv(const char *filename, Atom **atomsOut, int atomsCount);
int selectCell(int atomId, int atomsCount, Grid* grid, GridCell gridCell);
Grid* formGrid(Atom* atoms, int atomsCount, Substract substract, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms);
void findNeighborsInCell(Grid* grid, GridCell* cell, int cellId, Atom* neighbors[]);
void findNeighborsInNearCells(Grid* grid, int cellId, Atom* atom, Atom* neighbors[], int* curNeighbors);
int getNearCells(Grid* grid, int cellId, int cellNeighbors[]);
int xyzToId(int x, int y, int z, int Nx, int Ny, int Nz);

int main() {
    int atomsX = 6, atomsY = 4, atomsZ = 10;
    int atomsCount = atomsX * atomsY * atomsZ;
    atomsCount = 240;

    double radius = 2.5;

    Substract substruct;
    substruct.x = 20;
    substruct.y = 20;
    substruct.z = 10;

    int xCells = 2;
    int yCells = 2;
    int zCells = 5;

    Atom *atoms;
    // atoms = getAtoms(atomsCount);
    int realCount = read_csv("atom_positions.csv", &atoms, atomsCount);
    if (realCount != atomsCount) {
        printf("atoms in file is not the same as predicted\n");
        printf("in file: %d, predicted: %d\n", realCount, atomsCount);
    }

    Grid* grid = formGrid(atoms, atomsCount, substruct, xCells, yCells, zCells, atomsX, atomsY, atomsZ);
    Atom** nghbrs;
    nghbrs = findNeighbors(grid);

    for (int i = 0; i < 0; i++) {
        if (nghbrs[i]->preNeighbors != nghbrs[i]->neighbors) {
            printf("WARNING: neighbors count for atom with id:%d is not as prepared\n", atoms[i].id);
        }
    }
    return 0;
}

Grid* formGrid(Atom* atoms, int atomsCount, Substract substract, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms) {
    Grid* grid;
    GridCell gridCell;
    grid->xCellsCount = xCells;
    grid->yCellsCount = yCells;
    grid->zCellsCount = zCells;
    grid->atomsCount = atomsCount;
    gridCell.xAtomsCount = xAtoms;
    gridCell.yAtomsCount = yAtoms;
    gridCell.zAtomsCount = zAtoms;
    int cellsCount = xCells * yCells * zCells;
    int atomsPerCell = atomsCount / cellsCount;
    if (atomsCount % cellsCount != 0) {
        printf("WARNING: atomsCount %% cellsCount != 0");
    }
    grid->cells = calloc(xCells * yCells * zCells, sizeof(GridCell));
    for (int i = 0; i < atomsCount; i++) {
        int cellId = selectCell(i, atomsCount, grid, gridCell);
        int curAtomsCount = grid->cells[cellId].atomsCount;
        grid->cells[cellId].atoms[curAtomsCount] = atoms[i];
        grid->cells[cellId].atomsCount++;
    }
    return grid;
}

int selectCell(int atomId, int atomsCount, Grid* grid, GridCell gridCell) {
    int xyId = atomId;
    int zCellLayer = atomId / (gridCell.xAtomsCount * gridCell.yAtomsCount * grid->zCellsCount);
    int atomIdInLayerXY = atomId - atomId / (gridCell.xAtomsCount);
    int xCellLineInLayer = (atomIdInLayerXY / gridCell.xAtomsCount) % grid->xCellsCount;
    int yCellLineInLayer = (atomIdInLayerXY / (gridCell.xAtomsCount * grid->xCellsCount * grid->yCellsCount));

    int cellId = xCellLineInLayer + yCellLineInLayer * grid->xCellsCount + zCellLayer * grid->xCellsCount * grid->yCellsCount;

    return cellId;
}

Atom** findNeighbors(Grid* grid) {
    Atom **neighbors = malloc(grid->atomsCount * sizeof(Atom*));
    for (int i = 0; i < grid->atomsCount; i++) {
        neighbors[i] = malloc(neighborsCount * sizeof(Atom));
    }
    int cellsCount = grid->xCellsCount * grid->yCellsCount * grid->zCellsCount;
    for (int i = 0; i < cellsCount; i++) {
        findNeighborsInCell(grid, &grid->cells[i], i, neighbors);
    }
    return neighbors;
}

void findNeighborsInCell(Grid* grid, GridCell* cell, int cellId, Atom* neighbors[]) {
    for (int i = 0; i < cell->atomsCount; i++) {
        int curNeighborsCount = 0;
        for (int j = 0; j < cell->atomsCount; j++) {
            if (i != j && isNeighbor(cell->atoms[i], cell->atoms[j])) {
                neighbors[cell->atoms->id][curNeighborsCount] = cell->atoms[j];
                curNeighborsCount++;
            }
        }
        if (curNeighborsCount < 4) {
            findNeighborsInNearCells(grid, cellId, &cell->atoms[i], neighbors, &curNeighborsCount);
        }
        cell->atoms[i].neighbors = curNeighborsCount;
    }
}

void findNeighborsInNearCells(Grid* grid, int cellId, Atom* atom, Atom* neighbors[], int* curNeighbors) {
    int* cellNeighbors = malloc(26 * sizeof(int));
    int cellNeighborsCount = getNearCells(grid, cellId, cellNeighbors);
    for (int i = 0; i < cellNeighborsCount; i++) {
        for (int j = 0; j < grid->cells[cellNeighbors[i]].atomsCount; j++) {
            if (isNeighbor(*atom, grid->cells[cellNeighbors[i]].atoms[j])) {
                neighbors[*curNeighbors] = &grid->cells[cellNeighbors[i]].atoms[j];
                *curNeighbors++;
            }
        }
    }
}

int getNearCells(Grid* grid, int cellId, int cellNeighbors[]) {
    int cellNeighborsCount = 0;
    int x = cellId % grid->xCellsCount;
    int y = (cellId / grid->xCellsCount) % grid->yCellsCount;
    int z = cellId / (grid->xCellsCount * grid->yCellsCount);
    printf("cell id: %d, coords: x - %d, y - %d, z - %d", cellId, x, y, z);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dy == 0 && dz == 0) {
                    continue;
                }
                int nx = x + dx;
                int ny = y + dy;
                int nz = z + dz;

                if (nx >= 0 && nx < grid->xCellsCount &&
                    ny >= 0 && ny < grid->yCellsCount &&
                    nz >= 0 && nz < grid->zCellsCount) {
                    int neighborId = xyzToId(nx, ny, nz, grid->xCellsCount, grid->yCellsCount, grid->zCellsCount);
                    cellNeighbors[cellNeighborsCount] = neighborId;
                    cellNeighborsCount++;
                }
            }
        }
    }
    return cellNeighborsCount;
}

int xyzToId(int x, int y, int z, int Nx, int Ny, int Nz) {
    return x + y * Nx + z * Nx * Ny;
}

int isNeighbor(Atom a, Atom b) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    double z = a.z - b.z;
    double r = sqrt(x * x + y * y + z * z);
    // printf("%f\n", r);
    if (r <= neighborRadius) {
        return 1;
    }
    return 0;
}


int read_csv(const char *filename, Atom **atomsOut, int atomsCount) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Ошибка открытия файла");
        return -1;
    }

    char line[256];
    int count = 0;
    Atom *atoms = malloc(atomsCount * sizeof(Atom));
    if (!atoms) {
        fclose(f);
        return -1;
    }

    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        free(atoms);
        return -1;
    }
    int i = 0;
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;

        Atom a;
        a.id = i;
        i++;
        if (sscanf(line, "%lf,%lf,%lf,%d", &a.x, &a.y, &a.z, &a.preNeighbors) == 4) {
            if (count >= atomsCount) {
                atomsCount *= 2;
                atoms = realloc(atoms, atomsCount * sizeof(Atom));
                if (!atoms) {
                    fclose(f);
                    return -1;
                }
            }
            atoms[count++] = a;
        }
    }

    fclose(f);
    *atomsOut = atoms;
    return count;
}

Atom* getAtoms(int count) {
    Atom *atoms = malloc(count * sizeof(Atom));
    for (int i = 0; i < count; i++) {
        atoms[i].id = i;
        atoms[i].x = i % 20;
        atoms[i].y = i % 20;
        atoms[i].z = i % 20;
    }
    return atoms;
}
