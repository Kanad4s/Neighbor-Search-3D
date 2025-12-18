#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "utils.h"
#include "mpi.h"

const int NEIGHBORS_COUNT_MAX = 4;
const double NEIGHBOR_RADIUS = 2.5;
const int MAX_CELL_NEIGHBORS_COUNT = 26;
char *WRITE_FILE_NAME = "gridMpi.csv";

void findNeighbors(Grid *grid, NeighborList *neighbors);
Grid* formGrid(Atom *atoms, int atomsCount, int xCells, int yCells, int zCells, Substract substract);
void findNeighborsInCell(Grid *grid, GridCell *cell, int cellId, NeighborList *neighbors);
void findNeighborsInNearCells(Grid *grid, int cellId, Atom atom, NeighborList *neighbors);
int getNearCellsIDs(Grid *grid, int cellId, int *cellNeighbors);
int isCellIdAdded(int *cellNeighborsIDS, int cellNeighbors, int cellID);

int main(int argc, char *argv[]) {
    int rank;
    int nProcesses;
    Atom* atoms;
    NeighborList *neighbors;
    Grid* grid;
    int atomsCount = 18389;
    int cellsX = 20;
    int cellsY = 20;
    int cellsZ = 2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    if (rank == 0) {
        if (argc < 2) {
            printf("Usage: %s <filename>\n", argv[0]);
            return 1;
        }
        printf("Passed file: %s\n", argv[1]);
        // int cellsX = 17;
        // int cellsY = 17;
        // int cellsZ = 2;

        int cellsCount = cellsX * cellsY * cellsZ;
        printf("Cells: %d\n", cellsX * cellsY * cellsZ);
        Substract substract;
        int realCount = read_cls_with_bounds(argv[1], &atoms, atomsCount, &substract);
        printf("minX: %lf, maxX %lf, minY: %lf, maxY %lf, minZ: %lf, maxZ %lf\n", 
            substract.minX, substract.maxX, substract.minY, substract.maxY, substract.minZ, substract.maxZ);
        if (realCount != atomsCount) {
            printf("WARNING: atoms count in file %d != %d atoms expected\n", realCount, atomsCount);
            // return 0; 
        }

        grid = formGrid(atoms, realCount, cellsX, cellsY, cellsZ, substract);
        printf("grid formed\n");
        // printGrid(grid);
        for (int i = 0; i < grid->xCellsCount * grid->yCellsCount * grid->zCellsCount; i++) {
            // printf("cell: %d has atoms: %d\n", i, grid->cells[i].atomsCount);
        }
        
        neighbors = malloc(grid->atomsCount * sizeof(NeighborList));
        for (int i = 0; i < grid->atomsCount; i++) {
            neighbors[i].ids = malloc(NEIGHBORS_COUNT_MAX * sizeof(int));
            neighbors[i].count = 0;
        }
    }

    if (rank == 0) {
        findNeighbors(grid, neighbors);

    }


    if (rank == 0) {
        writeFile(atoms, neighbors, atomsCount, WRITE_FILE_NAME);
        free(neighbors);
    }

    MPI_Finalize();
    return 0;
}

Grid* formGrid(Atom* atoms, int atomsCount, int xCells, int yCells, int zCells, Substract substract) {
    double cellSizeX = (substract.maxX - substract.minX) / xCells;
    double cellSizeY = (substract.maxY - substract.minY) / yCells;
    double cellSizeZ = (substract.maxZ - substract.minZ) / zCells;

    Grid* grid = malloc(sizeof(Grid));
    int cellsCount = xCells * yCells * zCells;
    int atomsPerCell = atomsCount / cellsCount;
    // printf("Cells: %d\n", cellsCount);
    printf("Atoms per cell: %d\n", atomsPerCell);
    grid->xCellsCount = xCells;
    grid->yCellsCount = yCells;
    grid->zCellsCount = zCells;
    grid->atomsCount = atomsCount;
    
    if (atomsCount % cellsCount != 0) {
        printf("WARNING: atomsCount %% cellsCount = %d != 0\n", atomsCount % cellsCount);
        atomsPerCell++;
        printf("Updated atoms per cell: %d\n", atomsPerCell);
    }

    int totalCells = xCells * yCells * zCells;
    grid->cells = malloc(totalCells * sizeof(GridCell));

    for (int i = 0; i < totalCells; i++) {
        grid->cells[i].atomsCount = 0;
        grid->cells[i].atoms = malloc(((atomsCount / totalCells) * 2.1) * sizeof(Atom)); 
    }

    for (int i = 0; i < atomsCount; i++) {
        int cx = (int)((atoms[i].x - substract.minX) / cellSizeX);
        int cy = (int)((atoms[i].y - substract.minY) / cellSizeY);
        int cz = (int)((atoms[i].z - substract.minZ) / cellSizeZ);

        if (cx >= xCells) cx = xCells - 1;
        if (cy >= yCells) cy = yCells - 1;
        if (cz >= zCells) cz = zCells - 1;

        int cellId = cx + cy * xCells + cz * xCells * yCells;

        int idx = grid->cells[cellId].atomsCount;
        grid->cells[cellId].atoms[idx] = atoms[i];
        grid->cells[cellId].atomsCount++;
    }

    return grid;
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
        if (id == 18331) {
            printf("BEFORE FindNeighborsInNearCells, atomID: %d neighbors: ", id);
            for (int k = 0; k < neighbors[id].count; k++) {
                printf("neighbor: %d, ", neighbors[id].ids[k]);
            }
            printf("\n");
        }
        
        if (neighbors[id].count < NEIGHBORS_COUNT_MAX) {
            findNeighborsInNearCells(grid, cellId, cell->atoms[i], neighbors);
        }
        if (id == 18331) {
            printf("AFTER FindNeighborsInNearCells, atomID: %d neighbors: ", id);
            for (int k = 0; k < neighbors[id].count; k++) {
                printf("neighbor: %d, ", neighbors[id].ids[k]);
            }
            printf("\n");
        }
    }
}

void findNeighborsInNearCells(Grid *grid, int cellId, Atom atom, NeighborList *neighbors) {
    int *neighborCellsID = malloc(MAX_CELL_NEIGHBORS_COUNT * sizeof(int));
    int neighborCellsCount = getNearCellsIDs(grid, cellId, neighborCellsID);
    
    for (int i = 0; i < neighborCellsCount; i++) {
        for (int j = 0; j < grid->cells[neighborCellsID[i]].atomsCount; j++) {
            if (isNeighbor(atom, grid->cells[neighborCellsID[i]].atoms[j], NEIGHBOR_RADIUS)) {
                if (atom.id == 18331) {
                    printf("i: %d, j: %d\n", i, j);
                    printf("cell ID: %d, atomID: %d\n", neighborCellsID[i], grid->cells[neighborCellsID[i]].atoms[j].id);

                }
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

                int neighborCellID = convertCellCoordsToId(
                        nx, ny, nz,
                        grid->xCellsCount,
                        grid->yCellsCount,
                        grid->zCellsCount
                    );
                if (!isCellIdAdded(cellNeighborsIDS, count, neighborCellID)) {
                    cellNeighborsIDS[count++] = neighborCellID;
                }
                    
            }
        }
    }

    return count;
}

int isCellIdAdded(int *cellNeighborsIDS, int cellNeighbors, int cellID) {
    for (int i = 0; i < cellNeighbors; i++) {
        if (cellNeighborsIDS[i] == cellID) {
            return 1;
        }
    }

    return 0;
}

void idToXyz(int id, int Nx, int Ny, int Nz, int *x, int *y, int *z) {
    *z = id / (Nx * Ny);
    int rem = id % (Nx * Ny);
    *y = rem / Nx;
    *x = rem % Nx;
}