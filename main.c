#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    int id;
    int x, y, z;
} Atom;

int inBounds(int x, int y, int z, int gridX, int gridY, int gridZ) {
    return (x >= 0 && x <= gridX &&
            y >= 0 && y <= gridY &&
            z >= 0 && z <= gridZ);
}

// int neighbor(Atom a, int Lx, int Ly, int Lz, Atom neighbors[]) {
//     int count = 0;
//     for (int dx = -1; dx <= 1; dx++) {
//         for (int dy = -1; dy <= 1; dy++) {
//             for (int dz = -1; dz <= 1; dz++) {
//                 if (dx == 0 && dy == 0 && dz == 0) continue;

//                 int nx = a.x + dx;
//                 int ny = a.y + dy;
//                 int nz = a.z + dz;

//                 if (inBounds(nx, ny, nz, Lx, Ly, Lz)) {
//                     neighbors[count].x = nx;
//                     neighbors[count].y = ny;
//                     neighbors[count].z = nz;
//                     count++;
//                 }
//             }
//         }
//     }
//     return count;
// }

int isNeighbor(Atom a, Atom b, double radius);

int main() {
    int substrateX = 20, substrateY = 20, substrateZ = 10;
    int atomsCount = substrateX * substrateY * substrateZ;

    double radius = 2.5;

    Atom *atoms;
    // atoms = getAtoms(atomsCount);
    Atom a, b;
    a.x = 0,
    a.y = 0;
    a.z = 0;
    b.x = 2;
    b.y = 0;
    b.z = 1;

    printf("%d\n", isNeighbor(a, b, 2));

    // int **neighbors = findNeighbors(atoms, atomsCount, radius);

    // for (int i = 0; i < atomsCount; i++) {
    //     free(neighbors[i]);
    // }
    // free(neighbors);
    // free(atoms);

    return 0;
}

int** findNeighbors(Atom atoms[], int count, double radius) {
    Atom **neighbors = malloc(count * sizeof(Atom*));
    for (int i = 0; i < count; i++) {
        neighbors[i] = malloc((count - 1) * sizeof(Atom));
    }
    
    // for (int i = 0; i < count; i++) {
    //     Atom a = atoms[i];
    //     int n = neighbors(a, gridX, gridY, gridZ, neighbors);

    //     printf("Атом (%d, %d, %d), количество соседей: %d\n", a.x, a.y, a.z, n);
    //     for (int j = 0; j < n; j++) {
    //         printf("   -> (%d, %d, %d)\n", neighbors[j].x, neighbors[j].y, neighbors[j].z);
    //     }
    // }
}

int isNeighbor(Atom a, Atom b, double radius) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    double z = a.z - b.z;
    double r = sqrt(x * x + y * y + z * z);
    if (r <= radius) {
        return 1;
    }
    return 0;
}



// Atom* getAtoms(int count) {
//     Atom *atoms = malloc(count * sizeof(Atom));
//     for (int i = 0; i < count; i++) {
//         atoms[i].id = i;
//         atoms[i].x = i % 20;
//         atoms[i].y = i % 20;
//         atoms[i].z = i % 20;
//     }
//     return atoms;
// }
