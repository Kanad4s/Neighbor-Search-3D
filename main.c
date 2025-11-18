#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

const int NEIGHBORS_COUNT = 4;
const double NEIGHBOR_RADIUS = 2.5;

typedef struct {
    int id;
    double x, y, z;
    int neighbors;
    int preNeighbors;
} Atom;

int isNeighbor(Atom a, Atom b);
Atom* getAtoms(int count);
void findNeighbors(Atom *atoms, int count, Atom **neighbors);
int read_csv(const char *filename, Atom **atomsOut, int atomsCount);
void writeFile(Atom** neighbors, int atomsCount);

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

    writeFile(neighbors, realCount);
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

    for (int i = 0; i < 3; i++) {
        fgets(line, sizeof(line), f);
        printf("%s\n", line);
        
    }

    while (fgets(line, sizeof(line), f)) {
        Atom a;
        a.id = count;

        int n = sscanf(line, "%lf %lf %lf", &a.x, &a.y, &a.z);
        if (n == 3) {
            if (count >= atomsCount) {
                atomsCount *= 2;
                Atom *tmp = realloc(atoms, atomsCount * sizeof(Atom));
                if (!tmp) {
                    free(atoms);
                    fclose(f);
                    return -1;
                }
                atoms = tmp;
            }
            atoms[count] = a;
            count++;
        }
    }

    fclose(f);
    *atomsOut = atoms;
    return count;
}

void writeFile(Atom** neighbors, int atomsCount) {
    FILE *f = fopen("linearFinal.csv", "w");
    if (!f) {
        perror("Ошибка открытия файла");
        return;
    }

    fprintf(f, "id,x,y,z,neighbors\n");
    for (int i = 0; i < atomsCount; i++) {
        if (neighbors[i]->id > atomsCount || (neighbors[i]->x == 0.0 && neighbors[i]->y == 0.0 && neighbors[i]->z == 0.0)) {
            continue;
        }
        fprintf(f, "%d,%.6f,%.6f,%.6f,%d\n",
                neighbors[i]->id,
                neighbors[i]->x,
                neighbors[i]->y,
                neighbors[i]->z,
                neighbors[i]->neighbors);
    }

    fclose(f);
}
