#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include <time.h> 
#include <unistd.h>
#include <string.h>
#include "functions.h"


#define MAX_ITERATIONS 10000

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        perror("Possible wrong command input\nStandard: mpirun -n <number process> <compile file> <k-mean number> <number dimensions> <number seeds> \n");
        exit(1);
    }

    int k = atoi(argv[1]);          // number of clusters.
    int dimension = atoi(argv[2]);  // dimension of data.
    int totalPoint = atoi(argv[3]); // total point input
    double start, end;              //time start and end
    // Initial MPI and find process rank and number of processes.
    MPI_Init(NULL, NULL);
    int rank, sizeRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeRank);
    MPI_Barrier(MPI_COMM_WORLD);

    srand(time(NULL)); // Seed the random number generator to get different results each time
    // int totalPoint = 1000;
    int pointsPerProcess = totalPoint / sizeRank; //number points per process
    start = MPI_Wtime();                          //start count the time
    int counter = 0;                              //counter until 10000
    // - The points assigned to each cluster by each process.
    // - The points get back from each process.
    // - The current centroids.
    float *recvPoints, *points, *centroids;

    // - The number of point assigned to each cluster by each process.
    // - The labels for each cluster.
    int *counts, *labels;

    recvPoints = malloc(pointsPerProcess * dimension * sizeof(float)); // All points for all the processes.
    points = malloc(k * dimension * sizeof(float));                    // Sum of points assigned to each cluster by all processes.
    counts = malloc(k * sizeof(int));                                  // Sisze of each cluster
    centroids = malloc(k * dimension * sizeof(float));
    labels = malloc(pointsPerProcess * sizeof(int)); // The labels for each cluster.

    float *allPoints = NULL;   // All points for all the processes
    float *pointSums = NULL;   // Sum of points assigned to each cluster by all processes.
    int *clusterCounts = NULL; // Size of each cluster
    int *allLabels;            // Result of program: The labels for each cluster.

    if (rank == 0)
    {

        allPoints = createRandomNums(dimension * totalPoint); //create random number from 0 to 1
        // Take the first few k points as the initial cluster centroids.
        for (int i = 0; i < k * dimension; i++)
        {
            centroids[i] = allPoints[i];
        }

        inittialCentroids(centroids, k, dimension); //print centroids
        counter++;
        pointSums = malloc(k * dimension * sizeof(float));
        clusterCounts = malloc(k * sizeof(int));
        allLabels = malloc(sizeRank * pointsPerProcess * sizeof(int));
    }

    // Root sends each process its share of clusters.
    MPI_Scatter(allPoints, dimension * pointsPerProcess, MPI_FLOAT, recvPoints,
                dimension * pointsPerProcess, MPI_FLOAT, 0, MPI_COMM_WORLD);
    

    float distance = 1;

    while (distance > 0 && counter < MAX_ITERATIONS) //while counter less than 10000 or distance greater than 0.0001 do prcess n work
    {

        // Broadcast the current cluster centroids to all processes.
        MPI_Bcast(centroids, k * dimension, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Each process reinitializes its cluster accumulators.
        for (int i = 0; i < k * dimension; i++)
        {
            points[i] = 0.0;
        }

        for (int i = 0; i < k; i++)
        {
            counts[i] = 0;
        }

        // Find the closest centroid to each point and assign to cluster.
        float *pointsAssign = recvPoints;
        for (int i = 0; i < pointsPerProcess; i++, pointsAssign += dimension)
        {
            int clusterNum = assignLabel(pointsAssign, centroids, k, dimension);
            // Record the assignment of the site to the cluster.
            counts[clusterNum]++;                                               //increase size of point in this cluster
            addPoint(pointsAssign, &points[clusterNum * dimension], dimension); // add point into its cluster
        }

        // Gather and sum at root all cluster sums for individual processes.
        MPI_Reduce(points, pointSums, k * dimension, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        // Gather and sum count of point in each cluster
        MPI_Reduce(counts, clusterCounts, k, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0)
        {   //Calculate mean of each cluster by dividing sum of point with its number of points
            // Root process computes new centroids by dividing sums per cluster by count per cluster.
            for (int i = 0; i < k; i++)
            {
                for (int j = 0; j < dimension; j++)
                {

                    pointSums[dimension * i + j] /= clusterCounts[i];
                }
            }
            // Check if all mean are equally
            distance = distanceBetween(pointSums, centroids, dimension * k);
            printf("Current mean distance: %f\n", distance); //If mean distance is zero, It mean that all centroids have a been equally
            // Copy new centroids from pointSums into centroids.
            for (int i = 0; i < k * dimension; i++)
            {
                centroids[i] = pointSums[i];
            }
            notifyUpdateCentroids(centroids, k, dimension, &counter);
        }
        // Broadcast a better distance to all processes.
        MPI_Bcast(&distance, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
        counter++;
    }

    // Now centroids are fixed, so compute a final label for each cluster.
    float *pointsAssign = recvPoints;
    for (int i = 0; i < pointsPerProcess; i++, pointsAssign += dimension)
    {
        labels[i] = assignLabel(pointsAssign, centroids, k, dimension);
    }

    // Gather all labels into root process.
    MPI_Gather(labels, pointsPerProcess, MPI_INT,
               allLabels, pointsPerProcess, MPI_INT, 0, MPI_COMM_WORLD);

    // Root can print out all points and labels.
    if (rank == 0)
    {
        printf("\nFinal clustering result with centroid tag:\n");
        FILE *fp = fopen("./data/output.dat", "w");
        FILE *fp1 = fopen("./data/output1.dat", "w");
        FILE *fp2 = fopen("./data/output2.dat", "w");
        FILE *fpc = fopen("./data/final_centroids.dat", "w");
        for (size_t i = 0; i < k; i++)
        {
            for (size_t j = 0; j < dimension; j++, centroids++)
            {
                fprintf(fpc, "%f\t", *centroids);
            }
            fprintf(fpc, "\n");
        }

        float *allLabelPoints = allPoints;
        for (int i = 0; i < sizeRank * pointsPerProcess; i++, allLabelPoints += dimension)
        {

            for (int j = 0; j < dimension; j++)
            {
                printf("%f ", allLabelPoints[j]);

                //Only for 2k and 3k with 2 dimension
                if (allLabels[i] == 0)
                {
                    fprintf(fp, "%f\t", allLabelPoints[j]);
                }
                else if (allLabels[i] == 1)
                {
                    fprintf(fp1, "%f\t", allLabelPoints[j]);
                }
                else if (allLabels[i] == 2)
                {
                    fprintf(fp2, "%f\t", allLabelPoints[j]);
                }
            }
            //Only for 2k and 3k with 2 dimension
            if (allLabels[i] == 0)
            {
                fprintf(fp, "%4d\n", allLabels[i]);
            }
            else if (allLabels[i] == 1)
            {
                fprintf(fp1, "%4d\n", allLabels[i]);
            }
            else if (allLabels[i] == 2)
            {
                fprintf(fp2, "%4d\n", allLabels[i]);
            }

            printf("%4d\n", allLabels[i]);
        }

        fclose(fp);
        fclose(fp1);
        fclose(fpc);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    MPI_Finalize();
    if (rank == 0) /* use time on master node */
    {

        printf("Execution time %f \n", end - start);
    }
}
