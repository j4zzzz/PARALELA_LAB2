#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    int n = 4; 
    double *A = NULL, *x = NULL;
    double *local_A, *local_x, *local_y, *y;
    int local_cols;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (n % size != 0) {
        if (rank == 0) printf("n debe ser divisible por el numero de procesos\n");
        MPI_Finalize();
        return 0;
    }

    local_cols = n / size;  
    if (rank == 0) {
        A = malloc(n * n * sizeof(double));
        x = malloc(n * sizeof(double));
        y = malloc(n * sizeof(double));

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                A[i*n + j] = (i == j) ? 1.0 : 0.0;

        for (int i = 0; i < n; i++)
            x[i] = i + 1;
    }

    // Reservar espacio local
    local_A = malloc(n * local_cols * sizeof(double));
    local_x = malloc(local_cols * sizeof(double));
    local_y = malloc(n * sizeof(double));

    // Distribuir bloques de columnas de A
    MPI_Scatter(A, n*local_cols, MPI_DOUBLE,
                local_A, n*local_cols, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    MPI_Scatter(x, local_cols, MPI_DOUBLE,
                local_x, local_cols, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    for (int i = 0; i < n; i++)
        local_y[i] = 0.0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < local_cols; j++) {
            local_y[i] += local_A[i*local_cols + j] * local_x[j];
        }
    }

    int *recvcounts = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) recvcounts[i] = n / size;

    double *local_result = malloc(n/size * sizeof(double));

    MPI_Reduce_scatter(local_y, local_result, recvcounts,
                       MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    MPI_Gather(local_result, n/size, MPI_DOUBLE,
               y, n/size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Resultado y = [ ");
        for (int i = 0; i < n; i++) printf("%.1f ", y[i]);
        printf("]\n");
    }

    free(local_A); free(local_x); free(local_y);
    if (rank == 0) { free(A); free(x); free(y); }
    free(local_result); free(recvcounts);

    MPI_Finalize();
    return 0;
}
