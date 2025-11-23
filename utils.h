#ifndef UTILS_H
#define UTILS_H

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

void writeFile(Atom* atoms, Atom** neighbors, int atomsCount, char *filename);
int read_csv(const char *filename, Atom **atomsOut, int atomsCount);
int read_cls(const char *filename, Atom **atomsOut, int atomsCount, int cellsCount);
void printGrid(Grid* grid);
void printCell(GridCell cell, int x, int y, int z, int id);
void printAtom(Atom atom);

int convertCellCoordsToId(int x, int y, int z, int Nx, int Ny, int Nz);


#endif