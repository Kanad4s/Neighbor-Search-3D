#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include <string.h>

void writeFile(Atom* atoms, Atom** neighbors, int atomsCount, char *filename) {
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
        int curNeighborsCount = 0;
        fprintf(f, "atom:%d,%.6f,%.6f,%.6f,neighbors:%d, ", 
                atoms[i].id,
                atoms[i].x,
                atoms[i].y,
                atoms[i].z,
                atoms[i].neighbors);
        for (int j = 0; j < atoms[i].neighbors; j++) {
            curNeighborsCount = 0;
            fprintf(f, "id:%d,%.6f,%.6f,%.6f, ", 
                neighbors[i][j].id,
                neighbors[i][j].x,
                neighbors[i][j].y,
                neighbors[i][j].z);
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

     for (int i = 0; i < 3; i++) {
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
    return x + y * Nx + z * Nx * Ny;
}