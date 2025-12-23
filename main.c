#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "mpi.h"

const int NEIGHBORS_COUNT = 4;
const double NEIGHBOR_RADIUS = 2.5;
char *WRITE_FILE_NAME = "linear.csv";

void findNeighbors(Atom *atoms, int count, NeighborList *neighbors);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    printf("Passed file: %s\n", argv[1]);
    int atomsCount = 18389;

    double radius = 2.5;

    Atom *atoms;
    int realCount = read_csv(argv[1], &atoms, atomsCount);
    if (realCount != atomsCount) {
        printf("WARNING: atoms count in file %d != %d atoms expected\n", realCount, atomsCount);
        // return 0; 
    }
    NeighborList *neighbors = malloc(realCount * sizeof(NeighborList));
    for (int i = 0; i < realCount; i++) {
        neighbors[i].ids = malloc(NEIGHBORS_COUNT * sizeof(int));
        neighbors[i].count = 0;
    }
    
    double start = MPI_Wtime();
    findNeighbors(atoms, realCount, neighbors);
    double finish = MPI_Wtime();
    printf("TIME: %f\n", finish - start);

    writeFile(atoms, neighbors, realCount, WRITE_FILE_NAME);
    return 0;
}

void findNeighbors(Atom *atoms, int count, NeighborList *neighbors) { 
    for (int i = 0; i < count; i++) {
        Atom a = atoms[i];
        for (int j = 0; j < count; j++) {
            if (j == i) {
                continue;
            }
            if (isNeighbor(atoms[i], atoms[j], NEIGHBOR_RADIUS)) {
                neighbors[i].ids[neighbors[i].count] = atoms[j].id;
                neighbors[i].count++;
            }
        }
    }
}



