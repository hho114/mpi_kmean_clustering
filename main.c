#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
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

    
    int k = atoi(argv[1]);         // number of clusters.
    int dimension = atoi(argv[2]); // dimension of data.
    int numberOfSeed = atoi(argv[3]); // number of seeds per processor.
    double start, end; //time start and end
                       
    // Initial MPI and find process rank and number of processes.
    MPI_Init(NULL, NULL);
    int rank, sizeRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeRank);
    MPI_Barrier(MPI_COMM_WORLD);

    srand(time(NULL)); // Seed the random number generator to get different results each time

    start = MPI_Wtime();//start count the time

    float *seeds;
    seeds = malloc(numberOfSeed * dimension * sizeof(float));
    float *sums;     // The sum of seeds assigned to each cluster by this process.
    sums = malloc(k * dimension * sizeof(float));
    // The number of seeds assigned to each cluster by this process. k integers.
    int *counts;
    counts = malloc(k * sizeof(int));
    // The current centroids against which seeds are being compared.
    // These are shipped to the process by the root process.
    float *centroids;
    centroids = malloc(k * dimension * sizeof(float));
    // The cluster assignments for each site.
    int *labels;
    labels = malloc(numberOfSeed * sizeof(int));

    int counter = 0;
    
    // All the seeds for all the processes.
    // numberOfSeed * sizeRank vectors of d floats.
    float *allSeeds = NULL;
    // Sum of seeds assigned to each cluster by all processes.
    float *seedSums = NULL;
    // Number of seeds assigned to each cluster by all processes.
    int *seedCounts = NULL;
    // Result of program: a cluster label for each seed.
    int *allLabels;
    if (rank == 0)
    {
        allSeeds = createRandomNums(dimension * numberOfSeed * sizeRank);
        // Take the first k seeds as the initial cluster centroids.
        for (int i = 0; i < k * dimension; i++)
        {
            centroids[i] = allSeeds[i];
        }

        inittialCentroids(centroids, k, dimension);
        counter++;
        seedSums = malloc(k * dimension * sizeof(float));
        seedCounts = malloc(k * sizeof(int));
        allLabels = malloc(sizeRank * numberOfSeed * sizeof(int));
    }

    // Root sends each process its share of seeds.
    MPI_Scatter(allSeeds, dimension * numberOfSeed, MPI_FLOAT, seeds,
                dimension * numberOfSeed, MPI_FLOAT, 0, MPI_COMM_WORLD);

    float distance = 1.0; // Will tell us if centroids have moved.
    int counter = 0;
    while (distance > 0.001 && counter < MAX_ITERATIONS)
    { // While they've moved...

        // Broadcast the current cluster centroids to all processes.
        MPI_Bcast(centroids, k * dimension, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Each process reinitializes its cluster accumulators.
        for (int i = 0; i < k * dimension; i++)
            sums[i] = 0.0;
        for (int i = 0; i < k; i++)
            counts[i] = 0;

        // Find the closest centroid to each seed and assign to cluster.
        float *seed = seeds;
        for (int i = 0; i < numberOfSeed; i++, seed += dimension)
        {
            int cluster = assignSeed(seed, centroids, k, dimension);
            // Record the assignment of the site to the cluster.
            counts[cluster]++;
            addSeed(seed, &sums[cluster * dimension], dimension);
        }

        // Gather and sum at root all cluster sums for individual processes.
        MPI_Reduce(sums, seedSums, k * dimension, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(counts, seedCounts, k, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0)
        {
            // Root process computes new centroids by dividing sums per cluster
            // by count per cluster.
            for (int i = 0; i < k; i++)
            {
                for (int j = 0; j < dimension; j++)
                {
                    int dij = dimension * i + j;
                    seedSums[dij] /= seedCounts[i];
                }
            }
            // Have the centroids changed much?
            distance = distanceBetween(seedSums, centroids, dimension * k);
            printf("Better average distance: %f\n", distance);
            // Copy new centroids from seedSums into centroids.
            for (int i = 0; i < k * dimension; i++)
            {
                centroids[i] = seedSums[i];
            }
            notifyChangeCentroids(centroids, k, dimension, &counter);
        }
        // Broadcast a better distance.  All processes will use this in the loop test.
        MPI_Bcast(&distance, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
        counter++;
    }

    // Now centroids are fixed, so compute a final label for each site.
    float *seed = seeds;
    for (int i = 0; i < numberOfSeed; i++, seed += dimension)
    {
        labels[i] = assignSeed(seed, centroids, k, dimension);
    }

    // Gather all labels into root process.
    MPI_Gather(labels, numberOfSeed, MPI_INT,
               allLabels, numberOfSeed, MPI_INT, 0, MPI_COMM_WORLD);



    // Root can print out all seeds and labels.
    if ((rank == 0) && 1)
    {
        printf("\nFinal clustering result with centroid tag:\n"
               "x\ty\ttag\n");
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

        float *seed = allSeeds;
        for (int i = 0; i < sizeRank * numberOfSeed; i++, seed += dimension)
        {

            for (int j = 0; j < dimension; j++)
            {
                printf("%f ", seed[j]);
                
                //Only for 2k and 3k with 2 dimension
                if (allLabels[i] == 0)
                {
                    fprintf(fp, "%f\t", seed[j]);
                }
                else if (allLabels[i] == 1)
                {
                    fprintf(fp1, "%f\t", seed[j]);
                }
                else if (allLabels[i] == 2)
                {
                    fprintf(fp2, "%f\t", seed[j]);
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
