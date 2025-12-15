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


void findNeighbors(Grid *grid, NeighborList *neighbors);
int selectCell(int atomId, Grid* grid, int xAtomsPerCell, int yAtomsPerCell, int zAtomsPerCell);
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

    int cellsX = 2;
    int cellsY = 2;
    int cellsZ = 2;
    // int cellsX = 17;
    // int cellsY = 17;
    // int cellsZ = 2;

    int cellsCount = cellsX * cellsY * cellsZ;
    printf("Cells: %d\n", cellsX * cellsY * cellsZ);
    Atom* atoms;
    Substract substract;
    int realCount = read_cls_with_bounds(argv[1], &atoms, atomsCount, &substract);
    printf("minX: %lf, maxX %lf, minY: %lf, maxY %lf, minZ: %lf, maxZ %lf\n", 
        substract.minX, substract.maxX, substract.minY, substract.maxY, substract.minZ, substract.maxZ);
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
    int cellsCount = xCells * yCells * zCells;
    int atomsPerCell = atomsCount / cellsCount;
    // printf("Cells: %d\n", cellsCount);
    printf("Atoms per cell: %d\n", atomsPerCell);
    grid->xCellsCount = xCells;
    grid->yCellsCount = yCells;
    grid->zCellsCount = zCells;
    grid->atomsCount = atomsCount;
    int xAtomsPerCell = xAtoms / xCells;
    int yAtomsPerCell = yAtoms / yCells;
    int zAtomsPerCell = zAtoms / zCells;
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

    int* cellsChosen = calloc(cellsCount, sizeof(int));
    for (int i = 0; i < atomsCount; i++) {
        int cellId = selectCell(i, grid, xAtomsPerCell, yAtomsPerCell, zAtomsPerCell);
        cellsChosen[cellId]++;
        int curAtomsCount = grid->cells[cellId].atomsCount;
        grid->cells[cellId].atoms[curAtomsCount] = atoms[i];
        grid->cells[cellId].atomsCount += 1;
    }

    return grid;
}

int selectCell(int atomId, Grid* grid, int xAtomsPerCell, int yAtomsPerCell, int zAtomsPerCell) {
    int zCellLayer = atomId / (xAtomsPerCell * grid->xCellsCount * yAtomsPerCell * grid->yCellsCount * zAtomsPerCell);
    
    int atomIdInLayerXY = atomId % (xAtomsPerCell * grid->xCellsCount * yAtomsPerCell * grid->yCellsCount);

    int xCellLineInLayer = (atomIdInLayerXY / xAtomsPerCell) % grid->xCellsCount;
    int yCellLineInLayer = (atomIdInLayerXY / (xAtomsPerCell * grid->xCellsCount * yAtomsPerCell)) % grid->yCellsCount;

    int cellId = xCellLineInLayer + yCellLineInLayer * grid->xCellsCount + zCellLayer * grid->xCellsCount * grid->yCellsCount;

    return cellId;
}

void findNeighbors(Grid *grid, NeighborList *neighbors) {
    int cellsCount = grid->xCellsCount * grid->yCellsCount * grid->zCellsCount;
    for (int i = 0; i < cellsCount; i++) {
        findNeighborsInCell(grid, &grid->cells[i], i, neighbors);
    }
}

void findNeighborsInCell(Grid *grid, GridCell *cell, int cellId, NeighborList *neighbors) {
    for (int i = 0; i < cell->atomsCount; i++) {
        int id = cell->atoms[i].id;
        for (int j = 0; j < cell->atomsCount; j++) {
            if (i != j && isNeighbor(cell->atoms[i], cell->atoms[j], NEIGHBOR_RADIUS)) {
                if (neighbors[id].count < NEIGHBORS_COUNT_MAX) {
                    neighbors[id].ids[neighbors[id].count] = cell->atoms[j].id;
                    neighbors[id].count++;
                }
            }
        }
        
        if (neighbors[id].count < NEIGHBORS_COUNT_MAX) {
            findNeighborsInNearCells(grid, cellId, cell->atoms[i], neighbors);
        }
    }
}

void findNeighborsInNearCells(Grid *grid, int cellId, Atom atom, NeighborList *neighbors) {
    int *neighborCellsID = malloc(MAX_CELL_NEIGHBORS_COUNT * sizeof(int));
    int cellNeighborsCount = getNearCellsIDs(grid, cellId, neighborCellsID);
    
    for (int i = 0; i < cellNeighborsCount; i++) {
        for (int j = 0; j < grid->cells[neighborCellsID[i]].atomsCount; j++) {
            if (isNeighbor(atom, grid->cells[neighborCellsID[i]].atoms[j], NEIGHBOR_RADIUS)) {
                if (neighbors[atom.id].count < NEIGHBORS_COUNT_MAX) {
                    neighbors[atom.id].ids[neighbors[atom.id].count] = grid->cells[neighborCellsID[i]].atoms[j].id;
                    neighbors[atom.id].count++;
                }
            }
        }
    }
    free(neighborCellsID);
}

int getNearCellsIDs(Grid *grid, int cellId, int *cellNeighborsIDS) {
    int count = 0;

    int layerSize = grid->xCellsCount * grid->yCellsCount;

    int z = cellId / layerSize;
    int rem = cellId % layerSize;
    int y = rem / grid->xCellsCount;
    int x = rem % grid->xCellsCount;

    for (int dz = -1; dz <= 1; dz++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {

                if (dx == 0 && dy == 0 && dz == 0)
                    continue;

                int nx = (x + dx + grid->xCellsCount) % grid->xCellsCount;
                int ny = (y + dy + grid->yCellsCount) % grid->yCellsCount;
                int nz = (z + dz + grid->zCellsCount) % grid->zCellsCount;

                cellNeighborsIDS[count++] =
                    convertCellCoordsToId(
                        nx, ny, nz,
                        grid->xCellsCount,
                        grid->yCellsCount,
                        grid->zCellsCount
                    );
            }
        }
    }

    return count;
}

void idToXyz(int id, int Nx, int Ny, int Nz, int *x, int *y, int *z) {
    *z = id / (Nx * Ny);
    int rem = id % (Nx * Ny);
    *y = rem / Nx;
    *x = rem % Nx;
}