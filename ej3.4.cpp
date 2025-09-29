#include <stdio.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char** argv) {
    int rank, comm_sz;
    int local_val, partner, step;
    int recv_val;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    local_val = rank + 1;

    int p = 1;
    while (p * 2 <= comm_sz) p *= 2;

    if (rank >= p) {
        MPI_Send(&local_val, 1, MPI_INT, rank - p, 0, MPI_COMM_WORLD);

        MPI_Finalize();
        return 0;
    }

    if (rank + p < comm_sz) {
        MPI_Recv(&recv_val, 1, MPI_INT, rank + p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_val += recv_val;
    }

    int log_p = (int)log2(p);
    for (step = 0; step < log_p; step++) {
        int distance = 1 << step;
        partner = rank ^ distance;

        MPI_Sendrecv(&local_val, 1, MPI_INT, partner, 0,
                     &recv_val, 1, MPI_INT, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        local_val += recv_val;
    }

    printf("Proceso %d: suma global = %d\n", rank, local_val);

    MPI_Finalize();
    return 0;
}
