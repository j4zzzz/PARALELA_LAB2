#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    int n = 16;
    double start, end;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (n % size != 0) {
        if (rank == 0) printf("El tamaño del vector debe ser divisible por el número de procesos\n");
        MPI_Finalize();
        return 0;
    }

    int local_n = n / size;
    int* block_data = (int*) malloc(local_n * sizeof(int));

    for (int i = 0; i < local_n; i++) {
        block_data[i] = rank * local_n + i;
    }

    int* cyclic_data = (int*) malloc(local_n * sizeof(int));
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    for (int i = 0; i < local_n; i++) {
        int global_index = rank * local_n + i;
        int dest = global_index % size;  
        int pos  = global_index / size; 
        if (dest == rank) {
            cyclic_data[pos] = block_data[i];
        } else {
            MPI_Send(&block_data[i], 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    }
    for (int i = 0; i < local_n; i++) {
        int global_index = i * size + rank;
        if (global_index < n) {
            MPI_Recv(&cyclic_data[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    if (rank == 0) printf("Tiempo BLOCK -> CYCLIC = %f segundos\n", end - start);

    int* block_data2 = (int*) malloc(local_n * sizeof(int));
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    for (int i = 0; i < local_n; i++) {
        int global_index = i * size + rank;
        int dest = global_index / local_n; int pos  = global_index % local_n;
        if (dest == rank) {
            block_data2[pos] = cyclic_data[i];
        } else {
            MPI_Send(&cyclic_data[i], 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    }
    for (int i = 0; i < local_n; i++) {
        int global_index = rank * local_n + i;
        MPI_Recv(&block_data2[i], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    if (rank == 0) printf("Tiempo CYCLIC -> BLOCK = %f segundos\n", end - start);


    /*
    for (int p = 0; p < size; p++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == p) {
            printf("Rank %d final data: ", rank);
            for (int i = 0; i < local_n; i++) printf("%d ", block_data2[i]);
            printf("\n");
        }
    }
    */

    free(block_data);
    free(cyclic_data);
    free(block_data2);

    MPI_Finalize();
    return 0;
}
