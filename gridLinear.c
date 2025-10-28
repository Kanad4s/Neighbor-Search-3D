#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

const int NEIGHBORS_COUNT = 4;
const double NEIGHBOR_RADIUS = 2.5;
const int MAX_CELL_NEIGHBORS_COUNT = 26;

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
int read_cls(const char *filename, Atom **atomsOut, int atomsCount);
int selectCell(int atomId, Grid* grid, GridCell gridCell);
Grid* formGrid(Atom* atoms, int atomsCount, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms);
void findNeighborsInCell(Grid* grid, GridCell* cell, int cellId, Atom* neighbors[]);
void findNeighborsInNearCells(Grid* grid, int cellId, Atom* atom, Atom* neighbors[], int* curNeighbors);
int getNearCellsIDs(Grid* grid, int cellId, int cellNeighbors[]);
int convertCellCoordsToId(int x, int y, int z, int Nx, int Ny, int Nz);
void printGrid(Grid* grid);
void printCell(GridCell cell, int x, int y, int z, int id);
void printAtom(Atom atom);

int main() {
    // размеры подложки в атомах
    int atomsX = 68, atomsY = 68, atomsZ = 4;
    // int atomsCount = atomsX * atomsY * atomsZ;
    int atomsCount = 18496;

    int xCells = 17;
    int yCells = 17;
    int zCells = 2;
    printf("Atoms: %d, cells: %d\n", atomsCount, xCells * yCells * zCells);
    Atom* atoms;
    // int realCount = read_csv("atom_positions_96.csv", &atoms, atomsCount);
    int realCount = read_cls("mdf_18000.cls", &atoms, atomsCount);
    if (realCount != atomsCount) {
        // printf("WARNING: atoms count in file %d != %d atoms expected\n", realCount, atomsCount);
        // return 0; 
    }
    Grid* grid = formGrid(atoms, atomsCount, xCells, yCells, zCells, atomsX, atomsY, atomsZ);

    Atom** neighbors;
    neighbors = findNeighbors(grid);

    for (int i = 0; i < 0; i++) {
        if (neighbors[i]->preNeighbors != neighbors[i]->neighbors) {
            printf("WARNING: neighbors count for atom with id:%d is not as prepared\n", atoms[i].id);
        }
    }
    return 0;
}

Grid* formGrid(Atom* atoms, int atomsCount, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms) {
    Grid* grid = malloc(sizeof(Grid));
    GridCell gridCell;
    int cellsCount = xCells * yCells * zCells;
    int atomsPerCell = atomsCount / cellsCount;
    printf("Cells: %d\n", cellsCount);
    printf("Atoms per cell: %d\n", atomsPerCell);
    grid->xCellsCount = xCells;
    grid->yCellsCount = yCells;
    grid->zCellsCount = zCells;
    grid->atomsCount = atomsCount;
    gridCell.xAtomsCount = xAtoms / xCells;
    gridCell.yAtomsCount = yAtoms / yCells;
    gridCell.zAtomsCount = zAtoms / zCells;
    if (atomsCount % cellsCount != 0) {
        printf("WARNING: atomsCount %% cellsCount == %d != 0\n", atomsCount % cellsCount);
        atomsPerCell++;
        printf("Updated atoms per cell: %d\n", atomsPerCell);
    }
    grid->cells = malloc(xCells * yCells * zCells * sizeof(GridCell));
    for (int i = 0; i < cellsCount; i++) {
        grid->cells[i].atomsCount = 0;
        grid->cells[i].atoms = (Atom*)malloc(atomsPerCell * sizeof(Atom));
    }
    printf("Start form grid\n");
    int* cellsChosen = calloc(cellsCount, sizeof(int));
    for (int i = 0; i < atomsCount; i++) {
        int cellId = selectCell(i, grid, gridCell);
        // printf("selected cell: %d for atom: %d\n", cellId, i);
        cellsChosen[cellId]++;
        int curAtomsCount = grid->cells[cellId].atomsCount;
        grid->cells[cellId].atoms[curAtomsCount] = atoms[i];
        grid->cells[cellId].atomsCount = curAtomsCount + 1;
    }
    printf("Finish form grid\n");
    // for (int i = 0; i < cellsCount; i++) {
    //     printf("Cell %d has %d atoms, in Grid have to be: %d\n", i, cellsChosen[i], grid->cells[i].atomsCount);
    // }
    return grid;
}

int selectCell(int atomId, Grid* grid, GridCell gridCell) {
    int zCellLayer = atomId / (gridCell.xAtomsCount * grid->xCellsCount * gridCell.yAtomsCount * grid->yCellsCount * gridCell.zAtomsCount);
    
    int atomIdInLayerXY = atomId % (gridCell.xAtomsCount * grid->xCellsCount * gridCell.yAtomsCount * grid->yCellsCount);

    int xCellLineInLayer = (atomIdInLayerXY / gridCell.xAtomsCount) % grid->xCellsCount;
    int yCellLineInLayer = (atomIdInLayerXY / (gridCell.xAtomsCount * grid->xCellsCount * gridCell.yAtomsCount)) % grid->yCellsCount;

    int cellId = xCellLineInLayer + yCellLineInLayer * grid->xCellsCount + zCellLayer * grid->xCellsCount * grid->yCellsCount;

    return cellId;
}

Atom** findNeighbors(Grid* grid) {
    Atom **neighbors = malloc(grid->atomsCount * sizeof(Atom*));
    for (int i = 0; i < grid->atomsCount; i++) {
        neighbors[i] = malloc(NEIGHBORS_COUNT * sizeof(Atom));
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
    int* cellNeighborsIDs = malloc(MAX_CELL_NEIGHBORS_COUNT * sizeof(int));
    int cellNeighborsCount = getNearCellsIDs(grid, cellId, cellNeighborsIDs);
    for (int i = 0; i < cellNeighborsCount; i++) {
        for (int j = 0; j < grid->cells[cellNeighborsIDs[i]].atomsCount; j++) {
            if (isNeighbor(*atom, grid->cells[cellNeighborsIDs[i]].atoms[j])) {
                neighbors[*curNeighbors] = &grid->cells[cellNeighborsIDs[i]].atoms[j];
                *curNeighbors++;
            }
        }
    }
}

int getNearCellsIDs(Grid* grid, int cellId, int cellNeighborsIDS[]) {
    int cellNeighborsCount = 0;
    int z = cellId / (grid->xCellsCount * grid->yCellsCount);
    int xyLayerPos = cellId % (grid->xCellsCount * grid->yCellsCount);
    int y = xyLayerPos / grid->xCellsCount;
    int x = xyLayerPos % grid->xCellsCount;
    // printf("cell id: %d, coords: x - %d, y - %d, z - %d", cellId, x, y, z);
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
                    int cellNeighborId = convertCellCoordsToId(nx, ny, nz, grid->xCellsCount, grid->yCellsCount, grid->zCellsCount);
                    cellNeighborsIDS[cellNeighborsCount] = cellNeighborId;
                    cellNeighborsCount++;
                }
            }
        }
    }
    return cellNeighborsCount;
}

void idToXyz(int id, int Nx, int Ny, int Nz, int *x, int *y, int *z) {
    *z = id / (Nx * Ny);
    int rem = id % (Nx * Ny);
    *y = rem / Nx;
    *x = rem % Nx;
}

int convertCellCoordsToId(int x, int y, int z, int Nx, int Ny, int Nz) {
    return x + y * Nx + z * Nx * Ny;
}

int isNeighbor(Atom a, Atom b) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    double z = a.z - b.z;
    double r = sqrt(x * x + y * y + z * z);
    // printf("%f\n", r);
    if (r <= NEIGHBOR_RADIUS) {
        return 1;
    }
    return 0;
}

int read_cls(const char *filename, Atom **atomsOut, int atomsCount) {
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

    while (fgets(line, sizeof(line), f)) {
        // Пропускаем пустые строки и строки, где нет цифр
        int has_digit = 0;
        for (int j = 0; line[j]; j++) {
            if (isdigit((unsigned char)line[j]) || line[j] == '-' || line[j] == '+') {
                has_digit = 1;
                break;
            }
        }
        if (!has_digit)
            continue;

        Atom a;
        a.id = count;

        // Считываем только первые 3 double из строки
        int n = sscanf(line, "%lf %lf %lf", &a.x, &a.y, &a.z);
        if (n == 3) {
            if (count >= atomsCount) {
                atomsCount *= 2;
                Atom *tmp = realloc(atoms, atomsCount * sizeof(Atom));
                if (!tmp) {
                    free(atoms);
                    fclose(f);
                    return -1;
                }
                atoms = tmp;
            }
            atoms[count++] = a;
        }
    }

    fclose(f);
    *atomsOut = atoms;
    return count;
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

void printGrid(Grid* grid) {
    for (int z = 0; z < grid->zCellsCount; z++) {
        for (int y = 0; y < grid->yCellsCount; y++) {
            for (int x = 0; x < grid->xCellsCount; x++) {
                int id = convertCellCoordsToId(x, y, z, grid->xCellsCount, grid->yCellsCount, grid->zCellsCount);
                printCell(grid->cells[id], x, y, z, id);
            }
        }
    }
}

void printCell(GridCell cell, int x, int y, int z, int id) {
    printf("Cell: %d, x: %d, y: %d, z: %d\nAtoms:\n", id, x, y, z);
    for (int i = 0; i < cell.atomsCount; i++) {
        printAtom(cell.atoms[i]);
    }
}

void printAtom(Atom atom) {
    printf("\tAtom: %d, x: %lf, y: %lf, z: %lf\n", atom.id, atom.x, atom.y, atom.z);
}