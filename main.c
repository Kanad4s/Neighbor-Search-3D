#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"

const int NEIGHBORS_COUNT = 4;
const double NEIGHBOR_RADIUS = 2.5;

int isNeighbor(Atom a, Atom b);
Atom* getAtoms(int count);
void findNeighbors(Atom *atoms, int count, Atom **neighbors);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    printf("Passed file: %s\n", argv[1]);
    int atomsCount = 18390;

    double radius = 2.5;

    Atom *atoms;
    int realCount = read_csv(argv[1], &atoms, atomsCount);
    if (realCount != atomsCount) {
        printf("WARNING: atoms count in file %d != %d atoms expected\n", realCount, atomsCount);
        // return 0; 
    }
    Atom** neighbors = malloc(realCount * sizeof(Atom *));
    for (int i = 0; i < realCount; i++) {
        neighbors[i] = malloc(NEIGHBORS_COUNT * sizeof(Atom));
    }
    
    findNeighbors(atoms, realCount, neighbors);
    int count = 0;
    for (int i = 0; i < realCount; i++) {
        for (int j = 0; j < NEIGHBORS_COUNT; j++) {
            if (neighbors[i][j].id != 0) {

            } else {
                // printf("atom with index %d is null\n", i);
            }
        }
        count++;
    }
    printf("count: %d\n", count);

    writeFile(atoms, neighbors, realCount);
    return 0;
}

void findNeighbors(Atom *atoms, int count, Atom **neighbors) { 
    for (int i = 0; i < count; i++) {
        Atom a = atoms[i];
        int curNeighborsCount = 0;
        // поиск соседей
        for (int j = 0; j < count; j++) {
            if (j == i) {
                continue;
            }
            if (isNeighbor(atoms[i], atoms[j])) {
                // if (curNeighborsCount >= 4) {
                //     // printf("WARNING: neighbors count for atom with id:%d is more than 4\n", atoms[i].id);
                //     // break;
                // }
                neighbors[i][curNeighborsCount] = atoms[j];
                curNeighborsCount++;
            }
        }
        atoms[i].neighbors = curNeighborsCount;
    }
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


