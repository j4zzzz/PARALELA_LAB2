#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank==0) fprintf(stderr, "Uso: mpirun -np P ./matvec_block_submatrix N\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int n = atoi(argv[1]);
    int q = (int)round(sqrt((double)size));
    if (q*q != size) {
        if (rank==0) fprintf(stderr, "El número de procesos debe ser un cuadrado perfecto.\n");
        MPI_Abort(MPI_COMM_WORLD, 2);
    }
    if (n % q != 0) {
        if (rank==0) fprintf(stderr, "n debe ser divisible por sqrt(comm_sz)=%d\n", q);
        MPI_Abort(MPI_COMM_WORLD, 3);
    }

    int rows_per_block = n / q;
    int r = rank / q; 
    int c = rank % q; 
    
    double *Ablock = malloc(rows_per_block * rows_per_block * sizeof(double));
    double *xseg = malloc(rows_per_block * sizeof(double));
    double *y_partial = calloc(rows_per_block, sizeof(double));
    double *y_block = NULL;
    
    if (rank == 0) {
        double *A = malloc(n * n * sizeof(double));
        double *x = malloc(n * sizeof(double));
        for (int i = 0; i < n; ++i) {
            x[i] = (double)(i+1);
            for (int j = 0; j < n; ++j) {
                A[i*n + j] = (double)(i*n + j + 1);
            }
        }

        for (int pr = 0; pr < q; ++pr) {
            for (int pc = 0; pc < q; ++pc) {
                int dest = pr * q + pc;
                double *tmp = malloc(rows_per_block * rows_per_block * sizeof(double));
                for (int i = 0; i < rows_per_block; ++i) {
                    int global_i = pr * rows_per_block + i;
                    for (int j = 0; j < rows_per_block; ++j) {
                        int global_j = pc * rows_per_block + j;
                        tmp[i*rows_per_block + j] = A[global_i * n + global_j];
                    }
                }
                if (dest == 0) {
                    for (int i=0;i<rows_per_block*rows_per_block;++i) Ablock[i]=tmp[i];
                } else {
                    MPI_Send(tmp, rows_per_block*rows_per_block, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
                }
                free(tmp);
            }
        }

        for (int pc = 0; pc < q; ++pc) {
            int dest = pc * q + pc; 
            double *x_tmp = &x[pc * rows_per_block];
            if (dest == 0) {
                for (int i=0;i<rows_per_block;++i) xseg[i] = x_tmp[i];
            } else {
                MPI_Send(x_tmp, rows_per_block, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
            }
        }

        free(A);
        free(x);
    } else {
        MPI_Recv(Ablock, rows_per_block*rows_per_block, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (r == c) {
            MPI_Recv(xseg, rows_per_block, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    MPI_Comm col_comm, row_comm;
    MPI_Comm_split(MPI_COMM_WORLD, c, r, &col_comm);
    MPI_Comm_split(MPI_COMM_WORLD, r, c, &row_comm);
    
    int col_root = c;
    MPI_Bcast(xseg, rows_per_block, MPI_DOUBLE, col_root, col_comm);

    for (int i = 0; i < rows_per_block; ++i) {
        double sum = 0.0;
        for (int j = 0; j < rows_per_block; ++j) {
            sum += Ablock[i*rows_per_block + j] * xseg[j];
        }
        y_partial[i] = sum;
    }

    int row_root = r;
    if (r == c) {
        y_block = calloc(rows_per_block, sizeof(double));
    }
    MPI_Reduce(y_partial, y_block, rows_per_block, MPI_DOUBLE, MPI_SUM, row_root, row_comm);

    if (r == c) {
        int diag_rank = rank;
        if (diag_rank == 0) {
            double *y = malloc(n * sizeof(double));
            for (int i=0;i<rows_per_block;++i) y[i] = y_block[i];
            for (int pr = 1; pr < q; ++pr) {
                int src = pr * q + pr; 
                MPI_Recv(&y[pr*rows_per_block], rows_per_block, MPI_DOUBLE, src, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            if (rank == 0) {
                printf("Resultado y = A*x (n=%d):\n", n);
                for (int i = 0; i < n; ++i) printf("%g ", y[i]);
                printf("\n");
            }
            free(y);
        } else {
            MPI_Send(y_block, rows_per_block, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
        }
    }

    free(Ablock);
    free(xseg);
    free(y_partial);
    if (y_block) free(y_block);
    MPI_Comm_free(&col_comm);
    MPI_Comm_free(&row_comm);
    MPI_Finalize();
    return 0;
}
