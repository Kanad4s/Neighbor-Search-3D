#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

const int neighborsCount = 4;
const double neighborRadius = 2.5;

typedef struct {
    int id;
    double x, y, z;
    int neighbors;
    int preNeighbors;
} Atom;

int isNeighbor(Atom a, Atom b);
Atom* getAtoms(int count);
Atom** findNeighbors(Atom atoms[], int count);
int read_csv(const char *filename, Atom **atomsOut, int atomsCount);

int main() {
    int atomsCount = 18000;

    double radius = 2.5;

    Atom *atoms;
    // atoms = getAtoms(atomsCount);
    // int realCount = read_csv("atom_positions_96.csv", &atoms, atomsCount);
    int realCount = read_csv("mdf_18000.cls", &atoms, atomsCount);
    if (realCount != atomsCount) {
        printf("WARNING: atoms count in file %d != %d atoms expected\n", realCount, atomsCount);
        return 0; 
    }
    Atom** nghbrs;
    nghbrs = findNeighbors(atoms, atomsCount);

    // for (int i = 0; i < 0; i++) {
    //     if (nghbrs[i]->preNeighbors != nghbrs[i]->neighbors) {
    //         printf("WARNING: neighbors count for atom with id:%d is not as prepared\n", atoms[i].id);
    //     }
    // }
    return 0;
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
                // if (curNeighborsCount >= 4) {
                //     // printf("WARNING: neighbors count for atom with id:%d is more than 4\n", atoms[i].id);
                //     // break;
                // }
                neighbors[i][curNeighborsCount] = atoms[j];
                curNeighborsCount++;
            }
        }
        atoms[i].neighbors = curNeighborsCount;

        // printf("Атом: %d (%lf, %lf, %lf), количество соседей: %d\n", a.id, a.x, a.y, a.z, curNeighborsCount);
        // for (int j = 0; j < curNeighborsCount; j++) {
        //     printf("   -> id:%d (%lf, %lf, %lf)\n",  neighbors[i][j].id, neighbors[i][j].x, neighbors[i][j].y, neighbors[i][j].z);
        // }
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

    while (fgets(line, sizeof(line), f)) {
        // Пропускаем пустые строки и строки, где нет цифр
        int has_digit = 0;
        for (int j = 0; line[j]; j++) {
            if (isdigit((unsigned char)line[j]) || line[j] == '-' || line[j] == '+') {
                has_digit = 1;
                break;
            }
        }
        if (!has_digit)
            continue;

        Atom a;
        a.id = count;

        // Считываем только первые 3 double из строки
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
