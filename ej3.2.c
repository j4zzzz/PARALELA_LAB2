#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char* argv[]) {
    long long int tosses, local_tosses;
    long long int number_in_circle = 0, local_in_circle = 0;
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Ingrese el número total de lanzamientos: ");
        fflush(stdout);
        scanf("%lld", &tosses);
    }

    // Broadcast del total de lanzamientos
    MPI_Bcast(&tosses, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    // Dividir lanzamientos entre procesos
    local_tosses = tosses / size;

    // Semilla distinta para cada proceso
    srand(time(NULL) + rank);

    for (long long int toss = 0; toss < local_tosses; toss++) {
        double x = (double)rand() / RAND_MAX * 2.0 - 1.0;
        double y = (double)rand() / RAND_MAX * 2.0 - 1.0;
        double dist_sq = x * x + y * y;
        if (dist_sq <= 1.0) local_in_circle++;
    }

    // Reduce para sumar todos los locales en el proceso 0
    MPI_Reduce(&local_in_circle, &number_in_circle, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double pi_estimate = 4.0 * (double)number_in_circle / (double)tosses;
        printf("Estimación de pi = %lf\n", pi_estimate);
    }

    MPI_Finalize();
    return 0;
}
