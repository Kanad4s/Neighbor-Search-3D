#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include <string.h>

void writeFile(Atom *atoms, NeighborList *neighbors, int atomsCount, char *filename) {
    printf("write into %s\n", filename);
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Ошибка открытия файла");
        return;
    }

    // fprintf(f, "id,x,y,z,neighbors\n");
    for (int i = 0; i < atomsCount; i++) {
        // if (neighbors[i]->id > atomsCount || (neighbors[i]->x == 0.0 && neighbors[i]->y == 0.0 && neighbors[i]->z == 0.0)) {
        //     continue;
        // }
        fprintf(f, "atom:%d,%.6f,%.6f,%.6f,neighbors:%d, ", 
                atoms[i].id,
                atoms[i].x,
                atoms[i].y,
                atoms[i].z,
                neighbors[i].count);
        for (int j = 0; j < neighbors[i].count; j++) {
            fprintf(f, "id:%d,%.6f,%.6f,%.6f, ", 
                atoms[neighbors[i].ids[j]].id,
                atoms[neighbors[i].ids[j]].x,
                atoms[neighbors[i].ids[j]].y,
                atoms[neighbors[i].ids[j]].z);
        }
        fprintf(f, "\n");
        // printf("atoms.neighbors %d, countedNeighbors: %d\n", atoms[i].neighbors, curNeighborsCount);
    }

    fclose(f);
}

int read_cls(const char *filename, Atom **atomsOut, int atomsCount, int cellsCount) {
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

     for (int i = 0; i < 4; i++) {
        fgets(line, sizeof(line), f);
        // printf("%s\n", line);
    }

    while (fgets(line, sizeof(line), f)) {
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
        // usleep(atomsCount / 200 + cellsCount / 20);

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

int read_cls_with_bounds(const char *filename, Atom **atomsOut, int atomsCount,Substract *substract) {
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

    for (int i = 0; i < 4; i++) fgets(line, sizeof(line), f);

    int firstAtom = 1;
    while (fgets(line, sizeof(line), f)) {
        int has_digit = 0;
        for (int j = 0; line[j]; j++) {
            if (isdigit((unsigned char)line[j]) || line[j] == '-' || line[j] == '+') {
                has_digit = 1;
                break;
            }
        }
        if (!has_digit) continue;

        Atom a;
        a.id = count;
        int n = sscanf(line, "%lf %lf %lf", &a.x, &a.y, &a.z);
        if (n != 3) continue;

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

        if (firstAtom) {
            substract->minX = substract->maxX = a.x;
            substract->minY = substract->maxY = a.y;
            substract->minZ = substract->maxZ = a.z;
            firstAtom = 0;
        } else {
            if (a.x < substract->minX) substract->minX = a.x;
            if (a.x > substract->maxX) substract->maxX = a.x;
            if (a.y < substract->minY) substract->minY = a.y;
            if (a.y > substract->maxY) substract->maxY = a.y;
            if (a.z < substract->minZ) substract->minZ = a.z;
            if (a.z > substract->maxZ) substract->maxZ = a.z;
        }
    }

    fclose(f);
    *atomsOut = atoms;
    return count;
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

    for (int i = 0; i < 4; i++) {
        fgets(line, sizeof(line), f);
        // printf("%s\n", line);
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

void printGrid(Grid* grid) {
    for (int z = 0; z < grid->zCellsCount; z++) {
        for (int y = 0; y < grid->yCellsCount; y++) {
            for (int x = 0; x < grid->xCellsCount; x++) {
                int id = convertCellCoordsToId(x, y, z, grid->xCellsCount, grid->yCellsCount, grid->zCellsCount);
                printCell(grid->cells[id], x, y, z, id);
            }
        }
    }
}

void printCell(GridCell cell, int x, int y, int z, int id) {
    printf("Cell: %d, x: %d, y: %d, z: %d\nAtoms:\n", id, x, y, z);
    for (int i = 0; i < cell.atomsCount; i++) {
        printAtom(cell.atoms[i]);
    }
}

void printAtom(Atom atom) {
    printf("\tAtom: %d, x: %lf, y: %lf, z: %lf\n", atom.id, atom.x, atom.y, atom.z);
}

int convertCellCoordsToId(int x, int y, int z, int Nx, int Ny, int Nz) {
    // printf("convertCellCoordsToID: x: %d, y: %d, z:%d, Nx: %d, Ny: %d, Nz: %d, ConvertedCellID: %d\n", x, y, z, Nx, Ny, Nz, x + y * Nx + z * Nx * Ny);
    return x + y * Nx + z * Nx * Ny;
}

int isNeighbor(Atom a, Atom b, double neighborRadius) {
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