#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_ATTEMPTS 200000

typedef struct {
    double x, y, z;
    int neighbors;
} Atom;

double rand_uniform(double max) {
    return (double)rand() / RAND_MAX * max;
}

int generate_atoms(int N, double L, double min_dist, double cutoff, Atom *atoms) {
    int placed = 0;
    int attempts = 0;

    while (placed < N && attempts < MAX_ATTEMPTS) {
        attempts++;
        double x = rand_uniform(L);
        double y = rand_uniform(L);
        double z = rand_uniform(L);

        int ok = 1;
        int neighs_new = 0;

        // проверка против уже размещённых
        for (int i = 0; i < placed; i++) {
            double dx = x - atoms[i].x;
            double dy = y - atoms[i].y;
            double dz = z - atoms[i].z;
            double d2 = dx*dx + dy*dy + dz*dz;

            if (d2 < min_dist*min_dist) {
                ok = 0;
                break;
            }
            if (d2 <= cutoff*cutoff) {
                if (atoms[i].neighbors + 1 > 4) {
                    ok = 0;
                    break;
                }
                neighs_new++;
                if (neighs_new > 4) {
                    ok = 0;
                    break;
                }
            }
        }

        if (!ok) continue;

        // добавить атом
        atoms[placed].x = x;
        atoms[placed].y = y;
        atoms[placed].z = z;
        atoms[placed].neighbors = neighs_new;

        // обновить соседей у уже размещённых
        for (int i = 0; i < placed; i++) {
            double dx = x - atoms[i].x;
            double dy = y - atoms[i].y;
            double dz = z - atoms[i].z;
            double d2 = dx*dx + dy*dy + dz*dz;
            if (d2 <= cutoff*cutoff) {
                atoms[i].neighbors++;
            }
        }

        placed++;
    }

    return placed;
}

int main() {
    srand((unsigned)time(NULL));

    // === ТУТ меняешь параметры ===
    int N = 150;             // число атомов
    double L = 20.0;         // размер куба
    double min_dist = 2.1;   // минимальное расстояние
    double cutoff = 2.5;     // радиус соседства

    Atom *atoms = malloc(N * sizeof(Atom));
    if (!atoms) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    int placed = generate_atoms(N, L, min_dist, cutoff, atoms);

    FILE *f = fopen("atom_positions.csv", "w");
    if (!f) {
        fprintf(stderr, "Не удалось открыть файл для записи\n");
        free(atoms);
        return 1;
    }

    fprintf(f, "x,y,z,neighbors\n");
    for (int i = 0; i < placed; i++) {
        fprintf(f, "%.6f,%.6f,%.6f,%d\n",
                atoms[i].x, atoms[i].y, atoms[i].z, atoms[i].neighbors);
    }
    fclose(f);

    printf("Запрошено: %d, размещено: %d\n", N, placed);
    printf("Файл сохранён: atom_positions.csv\n");

    free(atoms);
    return 0;
}
