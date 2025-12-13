#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "utils.h"

const int NEIGHBORS_COUNT_MAX = 4;
const double NEIGHBOR_RADIUS = 2.5;
const int MAX_CELL_NEIGHBORS_COUNT = 26;
char *WRITE_FILE_NAME = "gridLinear.csv";

typedef struct {
    double x, y, z;
} Substract;

void findNeighbors(Grid *grid, NeighborList *neighbors);
int selectCell(int atomId, Grid *grid, GridCell gridCell);
Grid* formGrid(Atom *atoms, int atomsCount, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms);
void findNeighborsInCell(Grid *grid, GridCell *cell, int cellId, NeighborList *neighbors);
void findNeighborsInNearCells(Grid *grid, int cellId, Atom atom, NeighborList *neighbors);
int getNearCellsIDs(Grid *grid, int cellId, int *cellNeighbors);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    printf("Passed file: %s\n", argv[1]);
    // размеры подложки в атомах
    int atomsX = 68, atomsY = 68, atomsZ = 4;
    // int atomsCount = atomsX * atomsY * atomsZ;
    int atomsCount = 18390;

    int cellsX = 17;
    int cellsY = 17;
    int cellsZ = 2;
    int cellsCount = cellsX * cellsY * cellsZ;
    printf("Cells: %d\n", cellsX * cellsY * cellsZ);
    Atom* atoms;
    int realCount = read_cls(argv[1], &atoms, atomsCount, cellsCount);
    if (realCount != atomsCount) {
        printf("WARNING: atoms count in file %d != %d atoms expected\n", realCount, atomsCount);
        // return 0; 
    }
    Grid* grid = formGrid(atoms, realCount, cellsX, cellsY, cellsZ, atomsX, atomsY, atomsZ);
    printf("grid formed\n");
    // printGrid(grid);
    
    NeighborList *neighbors = malloc(grid->atomsCount * sizeof(NeighborList));
    for (int i = 0; i < grid->atomsCount; i++) {
        neighbors[i].ids = malloc(NEIGHBORS_COUNT_MAX * sizeof(int));
        neighbors[i].count = 0;
    }
    findNeighbors(grid, neighbors);

    // for (int i = 0; i < 0; i++) {
    //     if (neighbors[i]->preNeighbors != neighbors[i]->neighbors) {
    //         printf("WARNING: neighbors count for atom with id:%d is not as prepared\n", atoms[i].id);
    //     }
    // }

    writeFile(atoms, neighbors, atomsCount, WRITE_FILE_NAME);
    return 0;
}

Grid* formGrid(Atom* atoms, int atomsCount, int xCells, int yCells, int zCells,
     int xAtoms, int yAtoms, int zAtoms) {
    Grid* grid = malloc(sizeof(Grid));
    GridCell gridCell;
    int cellsCount = xCells * yCells * zCells;
    int atomsPerCell = atomsCount / cellsCount;
    // printf("Cells: %d\n", cellsCount);
    printf("Atoms per cell: %d\n", atomsPerCell);
    grid->xCellsCount = xCells;
    grid->yCellsCount = yCells;
    grid->zCellsCount = zCells;
    grid->atomsCount = atomsCount;
    gridCell.xAtomsCount = xAtoms / xCells;
    gridCell.yAtomsCount = yAtoms / yCells;
    gridCell.zAtomsCount = zAtoms / zCells;
    if (atomsCount % cellsCount != 0) {
        printf("WARNING: atomsCount %% cellsCount = %d != 0\n", atomsCount % cellsCount);
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

void findNeighbors(Grid *grid, NeighborList *neighbors) {
    int cellsCount = grid->xCellsCount * grid->yCellsCount * grid->zCellsCount;
    for (int i = 0; i < cellsCount; i++) {
        findNeighborsInCell(grid, &grid->cells[i], i, neighbors);
        return;
    }
}

void findNeighborsInCell(Grid *grid, GridCell *cell, int cellId, NeighborList *neighbors) {
    for (int i = 0; i < cell->atomsCount; i++) {
        for (int j = 0; j < cell->atomsCount; j++) {
            if (i != j && isNeighbor(cell->atoms[i], cell->atoms[j], NEIGHBOR_RADIUS)) {
                neighbors[cell->atoms[i].id].ids[neighbors[i].count] = cell->atoms[j].id;
                neighbors[cell->atoms[i].id].count++;
            }
        }
        // if (cellId == 0) {
        //     printf("before atom: %d, count: %d\n", i, neighbors[cell->atoms[i].id].count);
        // }
        
        if (neighbors[i].count < NEIGHBORS_COUNT_MAX) {
            findNeighborsInNearCells(grid, cellId, cell->atoms[i], neighbors);
        }
        return;
        // if (cellId == 0) {
        //     printf("after atom: %d, count: %d\n", i, neighbors[cell->atoms[i].id].count);
        // }
    }
}

void findNeighborsInNearCells(Grid *grid, int cellId, Atom atom, NeighborList *neighbors) {
    int *neighborCellsID = malloc(MAX_CELL_NEIGHBORS_COUNT * sizeof(int));
    int cellNeighborsCount = getNearCellsIDs(grid, cellId, neighborCellsID);
    if (cellId == 0) {
        printf("cellNeighborsCount: %d, cell: %d\n", cellNeighborsCount, cellId);
        for (int i = 0; i < cellNeighborsCount; i++) {
            printf("%d\n", neighborCellsID[i]);
        }
    }
    return;
    for (int i = 0; i < cellNeighborsCount; i++) {
        for (int j = 0; j < grid->cells[neighborCellsID[i]].atomsCount; j++) {
            if (isNeighbor(atom, grid->cells[neighborCellsID[i]].atoms[j], NEIGHBOR_RADIUS)) {
                neighbors[atom.id].ids[neighbors[atom.id].count] = grid->cells[neighborCellsID[i]].atoms[j].id;
                neighbors[atom.id].count++;
            }
        }
    }
}

int getNearCellsIDs(Grid *grid, int cellId, int *cellNeighborsIDS) {
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
                
                printf("nx: %d, ny: %d, nz: %d\n", nx, ny, nz);
                if (nx == -1) {
                    nx = grid->xCellsCount - 1;
                } else if (nx == grid->xCellsCount) {
                    nx = 0;
                }
                if (ny == -1) {
                    ny = grid->yCellsCount - 1;
                } else if (ny == grid->yCellsCount) {
                    ny = 0;
                }
                if (nz == -1) {
                    nz = grid->zCellsCount - 1;
                } else if (nz == grid->zCellsCount) {
                    nz = 0;
                }
                printf("nx: %d, ny: %d, nz: %d\n", nx, ny, nz);
                int cellNeighborId = convertCellCoordsToId(nx, ny, nz, grid->xCellsCount, grid->yCellsCount, grid->zCellsCount);
                cellNeighborsIDS[cellNeighborsCount] = cellNeighborId;
                cellNeighborsCount++;
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