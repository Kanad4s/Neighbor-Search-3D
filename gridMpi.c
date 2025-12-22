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

void findNeighbors(Grid *grid, NeighborList *neighbors, int rank, int nProcesses);
Grid* formGrid(Atom *atoms, int atomsCount, int xCells, int yCells, int zCells, Substract substract);
void findNeighborsInCell(Grid *grid, GridCell *cell, int cellId, NeighborList *neighbors);
void findNeighborsInNearCells(Grid *grid, int cellId, Atom atom, NeighborList *neighbors);
int getNearCellsIDs(Grid *grid, int cellId, int *cellNeighbors);
int isCellIdAdded(int *cellNeighborsIDS, int cellNeighbors, int cellID);

int main(int argc, char *argv[]) {
    int rank, nProcesses;
    Atom *atoms;
    NeighborList *neighbors;
    Grid *grid;
    int atomsCount = 18389;
    int cellsX = 24, cellsY = 24, cellsZ = 2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    if(rank==0 && cellsX % nProcesses != 0){
        printf("ABORTED: cellsX not divisible by nProcesses\n");
        MPI_Finalize();
        return 1;
    }

    if(argc < 2){
        if(rank==0) printf("Usage: %s <filename>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    Substract substract;
    int realCount = 0;
    if(rank==0){
        realCount = read_cls_with_bounds(argv[1], &atoms, atomsCount, &substract);
        if(realCount != atomsCount)
            printf("Warning: atoms in file (%d) != expected (%d)\n", realCount, atomsCount);
    } else {
        atoms = malloc(atomsCount * sizeof(Atom));
    }

    MPI_Bcast(&substract, sizeof(Substract), MPI_BYTE, 0, MPI_COMM_WORLD);
    MPI_Bcast(atoms, atomsCount*sizeof(Atom), MPI_BYTE, 0, MPI_COMM_WORLD);

    grid = formGrid(atoms, atomsCount, cellsX, cellsY, cellsZ, substract);

    neighbors = malloc(grid->atomsCount * sizeof(NeighborList));
    for(int i=0;i<grid->atomsCount;i++){
        neighbors[i].ids = malloc(NEIGHBORS_COUNT_MAX*sizeof(int));
        neighbors[i].count = 0;
    }

    int cellsCount = grid->xCellsCount * grid->yCellsCount * grid->zCellsCount;
    int cellsPerProc = cellsCount / nProcesses;
    int startCell = rank * cellsPerProc;
    int endCell = (rank==nProcesses-1) ? cellsCount : startCell + cellsPerProc;

    for(int i=startCell;i<endCell;i++){
        findNeighborsInCell(grid, &grid->cells[i], i, neighbors);
    }

    int sendCount = grid->atomsCount * NEIGHBORS_COUNT_MAX;
    int *sendBuf = malloc(sendCount * sizeof(int));
    for(int i=0;i<grid->atomsCount;i++){
        for(int j=0;j<NEIGHBORS_COUNT_MAX;j++){
            sendBuf[i*NEIGHBORS_COUNT_MAX+j] = (j<neighbors[i].count) ? neighbors[i].ids[j] : -1;
        }
    }

    int *recvCounts=NULL, *displs=NULL, *recvBuf=NULL;
    if(rank==0){
        recvCounts = malloc(nProcesses*sizeof(int));
        displs = malloc(nProcesses*sizeof(int));
        for(int i=0;i<nProcesses;i++){
            recvCounts[i] = sendCount;
            displs[i] = (i==0)?0:displs[i-1]+recvCounts[i-1];
        }
        recvBuf = malloc(sendCount*nProcesses*sizeof(int));
    }

    MPI_Gatherv(sendBuf, sendCount, MPI_INT,
                recvBuf, recvCounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    if(rank==0){
        for(int i=0;i<grid->atomsCount;i++){
            neighbors[i].count=0;
            for(int j=0;j<NEIGHBORS_COUNT_MAX;j++){
                for(int p=0;p<nProcesses;p++){
                    int val = recvBuf[p*sendCount + i*NEIGHBORS_COUNT_MAX + j];
                    if(val!=-1 && neighbors[i].count<NEIGHBORS_COUNT_MAX)
                        neighbors[i].ids[neighbors[i].count++] = val;
                }
            }
        }

        writeFile(atoms, neighbors, grid->atomsCount, WRITE_FILE_NAME);
        free(recvBuf); free(recvCounts); free(displs);
    }

    free(sendBuf);
    for(int i=0;i<grid->atomsCount;i++) free(neighbors[i].ids);
    free(neighbors);
    free(atoms);
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
        grid->cells[i].id = i;
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

void findNeighbors(Grid *grid, NeighborList *neighbors, int rank, int nProcesses) {
    int cellsCount = grid->xCellsCount * grid->yCellsCount * grid->zCellsCount;
    int start = cellsCount / nProcesses * rank;
    int finish = cellsCount / nProcesses * (rank + 1);
    
    #pragma omp parallel for schedule(dynamic)
    for (int i = start; i < finish; i++) {
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