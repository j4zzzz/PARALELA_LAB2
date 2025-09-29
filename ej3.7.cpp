#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define NITER 10000   
int main(int argc, char** argv) {
    int rank, size;
    int msg = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            printf("Este programa necesita exactamente 2 procesos\n");
        }
        MPI_Finalize();
        return 0;
    }

    clock_t start_c, end_c;
    if (rank == 0) {
        start_c = clock();
        for (int i = 0; i < NITER; i++) {
            msg = i;
            MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        }
        end_c = clock();
        double tiempo_clock = ((double)(end_c - start_c)) / CLOCKS_PER_SEC;
        printf("Tiempo con clock(): %f segundos\n", tiempo_clock);
    } else {
        for (int i = 0; i < NITER; i++) {
            MPI_Recv(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        double start_mpi = MPI_Wtime();
        for (int i = 0; i < NITER; i++) {
            msg = i;
            MPI_Send(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        }
        double end_mpi = MPI_Wtime();
        printf("Tiempo con MPI_Wtime(): %f segundos\n", end_mpi - start_mpi);
    } else {
        for (int i = 0; i < NITER; i++) {
            MPI_Recv(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
