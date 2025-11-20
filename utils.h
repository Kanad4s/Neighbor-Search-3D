#ifndef UTILS_H
#define UTILS_H

typedef struct {
    int id;
    double x, y, z;
    int neighbors;
    int preNeighbors;
} Atom;

void writeFile(Atom* atoms, Atom** neighbors, int atomsCount);
int read_csv(const char *filename, Atom **atomsOut, int atomsCount);
int read_cls(const char *filename, Atom **atomsOut, int atomsCount, int cellsCount);


#endif