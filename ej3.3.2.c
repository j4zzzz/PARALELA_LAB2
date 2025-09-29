#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, comm_sz;
    int local_val, partner, step;
    int global_sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    local_val = rank + 1; 
    global_sum = local_val;

    for (step = 1; step < comm_sz; step *= 2) {
        if (rank % (2*step) == 0) {
            partner = rank + step;
            if (partner < comm_sz) {
                int temp;
                MPI_Recv(&temp, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                global_sum += temp;
            }
        } else {
            partner = rank - step;
            MPI_Send(&global_sum, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            break; 
        }
    }

    if (rank == 0) {
        printf("Suma global = %d\n", global_sum);
    }

    MPI_Finalize();
    return 0;
}
