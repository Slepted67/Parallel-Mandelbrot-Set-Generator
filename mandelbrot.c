#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

extern void matToImage(char* filename, int* mat, int* dims);

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int nx = 3000;
    int ny = 2000;
    int maxIter = 1275;

    double xStart = -2;
    double xEnd = 1;
    double yStart = -1;
    double yEnd = 1;

    int *matrix = NULL;
    if (rank == 0) {
        matrix = (int*)malloc(nx * ny * sizeof(int));
        if (matrix == NULL) {
            printf("Rank %d: Memory allocation failed for matrix\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    double start_time, end_time;
    if (rank == 0) {
        start_time = MPI_Wtime();
        int rows_distributed = 0;
        int done = 0;

        for (int i = 1; i < size && rows_distributed < ny; i++) {
            MPI_Send(&rows_distributed, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            rows_distributed++;
        }

        while (done < ny) {
            MPI_Status status;
            int completed_row;
            MPI_Recv(&completed_row, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&matrix[completed_row * nx], nx, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            done++;

            if (rows_distributed < ny) {
                MPI_Send(&rows_distributed, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                rows_distributed++;
            } else if (done >= ny) {
                int terminate = -1;
                for (int i = 1; i < size; i++) {
                    MPI_Send(&terminate, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                }
                break;
            }
        }

        end_time = MPI_Wtime();
        printf("Total MPI time: %f seconds\n", end_time - start_time);

        int dims[2] = {ny, nx};
        matToImage("mandelbrot.jpg", matrix, dims);
        free(matrix);

    } else {
        int start_row;
        double computation_time = 0.0, communication_time = 0.0;

        while (1) {
            double comm_start_time = MPI_Wtime();
            MPI_Recv(&start_row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            double comm_end_time = MPI_Wtime();
            communication_time += comm_end_time - comm_start_time;

            if (start_row == -1) break;

            int *local_row = (int*)malloc(nx * sizeof(int));
            if (!local_row) {
                printf("Worker %d: Memory allocation failed\n", rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            double comp_start_time = MPI_Wtime();

            #pragma omp parallel
            {
                #pragma omp for schedule(dynamic)
                for (int j = 0; j < nx; j++) {
                    double x0 = xStart + (1.0 * j / nx) * (xEnd - xStart);
                    double y0 = yStart + (1.0 * start_row / ny) * (yEnd - yStart);

                    double x = 0, y = 0;
                    int iter = 0;

                    while (iter < maxIter) {
                        double temp = x * x - y * y + x0;
                        y = 2 * x * y + y0;
                        x = temp;
                        iter++;
                        if (x * x + y * y > 4) break;
                    }
                    local_row[j] = iter;
                }
            }

            double comp_end_time = MPI_Wtime();
            computation_time += comp_end_time - comp_start_time;

            comm_start_time = MPI_Wtime();
            MPI_Send(&start_row, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(local_row, nx, MPI_INT, 0, 1, MPI_COMM_WORLD);
            comm_end_time = MPI_Wtime();
            communication_time += comm_end_time - comm_start_time;

            free(local_row);
        }

        printf("Worker %d: Finished all computation in %f seconds (computation)\n", rank, computation_time);
    }

    MPI_Finalize();
    return 0;
}
