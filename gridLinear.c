#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

const int neighborsCount = 4;
const double neighborRadius = 2.5;

typedef struct {
    int id;
    double x, y, z;
    int neighbors;
    int preNeighbors;
} Atom;

typedef struct {
    Atom* atoms;
    int atomsCount;
    double xAtomsCount, yAtomsCount, zAtomsCount;
} GridCell;

typedef struct {
    GridCell* cells;
    int xCellsCount, yCellsCount, zCellsCount;
} Grid;

typedef struct {
    double x, y, z;
} Substract;

int isNeighbor(Atom a, Atom b);
Atom* getAtoms(int count);
Atom** findNeighbors(Atom atoms[], int count);
int read_csv(const char *filename, Atom **atomsOut, int atomsCount);
int selectCell(int atomId, int atomsCount, Grid* grid);

int main() {
    int substrateX = 20, substrateY = 20, substrateZ = 10;
    int atomsCount = substrateX * substrateY * substrateZ;
    atomsCount = 150;

    double radius = 2.5;

    Substract substruct;
    substruct.x = 20;
    substruct.y = 20;
    substruct.z = 10;

    Atom *atoms;
    // atoms = getAtoms(atomsCount);
    int realCount = read_csv("atom_positions.csv", &atoms, atomsCount);
    


    Atom** nghbrs;
    nghbrs = findNeighbors(atoms, atomsCount);

    for (int i = 0; i < 0; i++) {
        if (nghbrs[i]->preNeighbors != nghbrs[i]->neighbors) {
            printf("WARNING: neighbors count for atom with id:%d is not as prepared\n", atoms[i].id);
        }
    }
    return 0;
}

Grid formGrid(Atom* atoms, int atomsCount, Substract substract, int xCells, int yCells, int zCells) {
    Grid* grid;
    grid->xCellsCount = xCells;
    grid->yCellsCount = yCells;
    grid->zCellsCount = zCells;
    int cellsCount = xCells * yCells * zCells;
    int atomsPerCell = atomsCount / cellsCount;
    if (atomsCount % cellsCount != 0) {
        printf("WARNING: atomsCount %% cellsCount != 0");
    }
    grid->cells = malloc(xCells * yCells * zCells * sizeof(GridCell));
    for (int i = 0; i < atomsCount; i++) {
        int cellId = selectCell(i, atomsCount, grid);
    }
}

int selectCell(int atomId, int atomsCount, Grid* grid) {
    int zLayer = atomId / (grid->xCellsCount * grid->yCellsCount);
    int cellsCount = grid->xCellsCount * grid->yCellsCount * grid->zCellsCount;
    int atomsPerCell = atomsCount / cellsCount;
    int cellId = atomsCount / atomsPerCell;



    int xGrid = atomId / grid->xCellsCount;

    // if (atomId)
}

Atom** findNeighbors(Atom atoms[], int count) {
    Atom **neighbors = malloc(count * sizeof(Atom*));
    for (int i = 0; i < count; i++) {
        neighbors[i] = malloc(neighborsCount * sizeof(Atom));
    }
    
    for (int i = 0; i < count; i++) {
        Atom a = atoms[i];
        int curNeighborsCount = 0;
        // поиск соседей
        for (int j = 0; j < count; j++) {
            if (j == i) {
                continue;
            }
            if (isNeighbor(atoms[i], atoms[j])) {
                if (curNeighborsCount >= 4) {
                    printf("WARNING: neighbors count for atom with id:%d is more than 4\n", atoms[i].id);
                    break;
                }
                neighbors[i][curNeighborsCount] = atoms[j];
                curNeighborsCount++;
            }
        }
        atoms[i].neighbors = curNeighborsCount;

        printf("Атом: %d (%lf, %lf, %lf), количество соседей: %d\n", a.id, a.x, a.y, a.z, curNeighborsCount);
        for (int j = 0; j < curNeighborsCount; j++) {
            printf("   -> id:%d (%lf, %lf, %lf)\n",  neighbors[i][j].id, neighbors[i][j].x, neighbors[i][j].y, neighbors[i][j].z);
        }
    }

    return neighbors;
}

int isNeighbor(Atom a, Atom b) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    double z = a.z - b.z;
    double r = sqrt(x * x + y * y + z * z);
    // printf("%f\n", r);
    if (r <= neighborRadius) {
        return 1;
    }
    return 0;
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
