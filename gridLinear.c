#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

const int NEIGHBORS_COUNT_MAX = 4;
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
int read_cls(const char *filename, Atom **atomsOut, int atomsCount, int cellsCount);
int selectCell(int atomId, Grid* grid, GridCell gridCell);
Grid* formGrid(Atom* atoms, int atomsCount, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms);
void findNeighborsInCell(Grid* grid, GridCell* cell, int cellId, Atom** neighbors);
int findNeighborsInNearCells(Grid* grid, int cellId, Atom* atom, Atom* neighbors[], int curNeighbors);
int getNearCellsIDs(Grid* grid, int cellId, int cellNeighbors[]);
int convertCellCoordsToId(int x, int y, int z, int Nx, int Ny, int Nz);
void printGrid(Grid* grid);
void printCell(GridCell cell, int x, int y, int z, int id);
void printAtom(Atom atom);
void writeFile(Atom** neighbors, int atomsCount);

int main() {
    // размеры подложки в атомах
    int atomsX = 68, atomsY = 68, atomsZ = 4;
    // int atomsCount = atomsX * atomsY * atomsZ;
    int atomsCount = 18496;

    int cellsX = 17;
    int cellsY = 17;
    int cellsZ = 2;
    int cellsCount = cellsX * cellsY * cellsZ;
    printf("Atoms: %d, cells: %d\n", atomsCount, cellsX * cellsY * cellsZ);
    Atom* atoms;
    int realCount = read_cls("mdf.cls", &atoms, atomsCount, cellsCount);
    if (realCount != atomsCount) {
        // printf("WARNING: atoms count in file %d != %d atoms expected\n", realCount, atomsCount);
        // return 0; 
    }
    Grid* grid = formGrid(atoms, atomsCount, cellsX, cellsY, cellsZ, atomsX, atomsY, atomsZ);
    
    Atom** neighbors;
    neighbors = findNeighbors(grid);

    // for (int i = 0; i < 0; i++) {
    //     if (neighbors[i]->preNeighbors != neighbors[i]->neighbors) {
    //         printf("WARNING: neighbors count for atom with id:%d is not as prepared\n", atoms[i].id);
    //     }
    // }

    // writeFile(neighbors, atomsCount);
    return 0;
}

Grid* formGrid(Atom* atoms, int atomsCount, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms) {
    Grid* grid = malloc(sizeof(Grid));
    GridCell gridCell;
    int cellsCount = xCells * yCells * zCells;
    int atomsPerCell = atomsCount / cellsCount;
    // printf("Cells: %d\n", cellsCount);
    // printf("Atoms per cell: %d\n", atomsPerCell);
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
    // printf("Start form grid\n");
    int* cellsChosen = calloc(cellsCount, sizeof(int));
    for (int i = 0; i < atomsCount; i++) {
        int cellId = selectCell(i, grid, gridCell);
        // printf("selected cell: %d for atom: %d\n", cellId, i);
        cellsChosen[cellId]++;
        int curAtomsCount = grid->cells[cellId].atomsCount;
        grid->cells[cellId].atoms[curAtomsCount] = atoms[i];
        grid->cells[cellId].atomsCount += 1;
    }
    // printf("Finish form grid\n");
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
    Atom **neighbors = malloc(grid->atomsCount * sizeof(Atom**));
    for (int i = 0; i < grid->atomsCount; i++) {
        neighbors[i] = malloc(NEIGHBORS_COUNT_MAX * sizeof(Atom*));
    }
    int cellsCount = grid->xCellsCount * grid->yCellsCount * grid->zCellsCount;
    for (int i = 0; i < cellsCount; i++) {
        findNeighborsInCell(grid, &grid->cells[i], i, neighbors);
    }
    return neighbors;
}

void findNeighborsInCell(Grid* grid, GridCell* cell, int cellId, Atom** neighbors) {
    for (int i = 0; i < cell->atomsCount; i++) {
        int curNeighborsCount = 0;
        for (int j = 0; j < cell->atomsCount; j++) {
            if (i != j && isNeighbor(cell->atoms[i], cell->atoms[j])) {
                neighbors[cell->atoms[i].id][curNeighborsCount] = cell->atoms[j];
                curNeighborsCount++;
            }
        }
        // printf("nghrb %d\n", curNeighborsCount);
        if (curNeighborsCount < NEIGHBORS_COUNT_MAX) {
            // printf("find nee\n");
            int nghbrsCount = findNeighborsInNearCells(grid, cellId, &cell->atoms[i], neighbors, curNeighborsCount);
            curNeighborsCount = nghbrsCount;
        }
        cell->atoms[i].neighbors = curNeighborsCount;
    }
}

int findNeighborsInNearCells(Grid* grid, int cellId, Atom* atom, Atom* neighbors[], int curNeighbors) {
    int* cellNeighborsIDs = malloc(MAX_CELL_NEIGHBORS_COUNT * sizeof(int));
    int cellNeighborsCount = getNearCellsIDs(grid, cellId, cellNeighborsIDs);
    for (int i = 0; i < cellNeighborsCount; i++) {
        for (int j = 0; j < grid->cells[cellNeighborsIDs[i]].atomsCount; j++) {
            if (isNeighbor(*atom, grid->cells[cellNeighborsIDs[i]].atoms[j])) {
                neighbors[curNeighbors] = &grid->cells[cellNeighborsIDs[i]].atoms[j];
                curNeighbors++;
            }
        }
    }
    return curNeighbors;
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

void writeFile(Atom** neighbors, int atomsCount) {
    FILE *f = fopen("cellsFinal.csv", "w");
    if (!f) {
        perror("Ошибка открытия файла");
        return;
    }

    fprintf(f, "id,x,y,z,neighbors\n");
    for (int i = 0; i < atomsCount; i++) {
        // if (neighbors[i]->id > atomsCount || (neighbors[i]->x == 0.0 && neighbors[i]->y == 0.0 && neighbors[i]->z == 0.0)) {
        //     printf("continue\n");
        //     continue;
        // }
        for (int j = 0; j < neighbors[i]->neighbors; j++) {
            fprintf(f, "%d,%.6f,%.6f,%.6f,%d\n",
                neighbors[i]->id,
                neighbors[i]->x,
                neighbors[i]->y,
                neighbors[i]->z,
                neighbors[i]->neighbors);
        }
        
    }

    fclose(f);
}