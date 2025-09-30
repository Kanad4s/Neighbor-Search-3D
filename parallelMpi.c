#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int worldSize;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    int worldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    char processorName[MPI_MAX_PROCESSOR_NAME];
    int nameLen;
    MPI_Get_processor_name(processorName, &nameLen);
    printf("Hello from processor %s, rank %d out of %d processors\n", processorName, worldRank, worldSize);
    MPI_Finalize();
    return 0;
}
