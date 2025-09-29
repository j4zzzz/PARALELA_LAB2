#include <stdio.h>
#include <mpi.h>

#define TOTAL_DATA_POINTS 20
#define NUMBER_OF_BINS 5

int find_bin(float value, float global_min, float global_max, int bin_count, double bin_width);

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    
    int process_rank, total_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    float all_data[] = {1.3, 2.9, 0.4, 0.3, 1.3, 4.4, 1.7, 0.4, 3.2, 0.3,
                        4.9, 2.4, 3.1, 4.4, 3.9, 0.4, 4.2, 4.5, 4.9, 0.9};

    int data_per_process = TOTAL_DATA_POINTS / total_processes;
    float local_data[data_per_process];

    MPI_Scatter(all_data, data_per_process, MPI_FLOAT,
                local_data, data_per_process, MPI_FLOAT,
                0, MPI_COMM_WORLD);

    float local_minimum = local_data[0];
    float local_maximum = local_data[0];
    for (int i = 1; i < data_per_process; i++) {
        if (local_data[i] < local_minimum) local_minimum = local_data[i];
        if (local_data[i] > local_maximum) local_maximum = local_data[i];
    }

    float global_minimum, global_maximum;
    MPI_Allreduce(&local_minimum, &global_minimum, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&local_maximum, &global_maximum, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

    double width_per_bin = (global_maximum - global_minimum) / NUMBER_OF_BINS;

    int local_histogram[NUMBER_OF_BINS];
    for (int i = 0; i < NUMBER_OF_BINS; i++) {
        local_histogram[i] = 0;
    }

    for (int i = 0; i < data_per_process; i++) {
        int bin_index = find_bin(local_data[i], global_minimum, global_maximum, NUMBER_OF_BINS, width_per_bin);
        local_histogram[bin_index]++;
    }

    int global_histogram[NUMBER_OF_BINS];
    MPI_Reduce(local_histogram, global_histogram, NUMBER_OF_BINS, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (process_rank == 0) {
        printf("Histogram Results:\n");
        for (int i = 0; i < NUMBER_OF_BINS; i++) {
            float bin_lower = global_minimum + i * width_per_bin;
            float bin_upper = global_minimum + (i + 1) * width_per_bin;
            printf("Bin %d [%.2f-%.2f): %d values\n", i, bin_lower, bin_upper, global_histogram[i]);
        }
    }

    MPI_Finalize();
    return 0;
}

int find_bin(float value, float global_min, float global_max, int bin_count, double bin_width) {
    int bin_index = (int)((value - global_min) / bin_width);
    
    if (bin_index == bin_count) {
        bin_index--;
    }
    
    return bin_index;
}