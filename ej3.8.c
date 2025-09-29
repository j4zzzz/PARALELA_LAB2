#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void merge(int* A, int sizeA, int* B, int sizeB, int* C) {
    int i=0, j=0, k=0;
    while (i < sizeA && j < sizeB) {
        if (A[i] <= B[j]) C[k++] = A[i++];
        else C[k++] = B[j++];
    }
    while (i < sizeA) C[k++] = A[i++];
    while (j < sizeB) C[k++] = B[j++];
}

int cmpfunc(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int main(int argc, char** argv) {
    int rank, size, n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Ingrese n (tamaño total, divisible por %d): ", size);
        fflush(stdout);
        scanf("%d", &n);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_n = n / size;
    int* local = (int*) malloc(local_n * sizeof(int));

    srand(rank + 1);
    for (int i = 0; i < local_n; i++) {
        local[i] = rand() % 100;
    }

    qsort(local, local_n, sizeof(int), cmpfunc);

    if (rank == 0) {
        int* temp = (int*) malloc(n * sizeof(int));
        for (int i = 0; i < local_n; i++)
            temp[i] = local[i];

        for (int p = 1; p < size; p++) {
            MPI_Recv(temp + p*local_n, local_n, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("Listas locales ordenadas:\n");
        for (int i = 0; i < n; i++) printf("%d ", temp[i]);
        printf("\n\n");
        free(temp);
    } else {
        MPI_Send(local, local_n, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    int step = 1;
    int* merged = local;
    int merged_size = local_n;

    while (step < size) {
        if (rank % (2*step) == 0) {
            if (rank + step < size) {
                int recv_size = merged_size;
                int* recv_data = (int*) malloc(recv_size * sizeof(int));

                MPI_Recv(&recv_size, 1, MPI_INT, rank+step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                recv_data = (int*) malloc(recv_size * sizeof(int));
                MPI_Recv(recv_data, recv_size, MPI_INT, rank+step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int* new_merge = (int*) malloc((merged_size+recv_size)*sizeof(int));
                merge(merged, merged_size, recv_data, recv_size, new_merge);

                if (merged != local) free(merged);
                free(recv_data);
                merged = new_merge;
                merged_size += recv_size;
            }
        } else {
            int dest = rank - step;
            MPI_Send(&merged_size, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(merged, merged_size, MPI_INT, dest, 0, MPI_COMM_WORLD);
            break; 
        }
        step *= 2;
    }

    if (rank == 0) {
        printf("Lista global ordenada:\n");
        for (int i = 0; i < merged_size; i++) printf("%d ", merged[i]);
        printf("\n");
    }

    if (merged != local) free(merged);
    else free(local);

    MPI_Finalize();
    return 0;
}
