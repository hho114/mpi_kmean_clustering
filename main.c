#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

// Creates an array of random floats. Each number has a value from 0 - 1
float *createRandomNums(const int num_elements)
{
    float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
    assert(rand_nums != NULL);
    for (int i = 0; i < num_elements; i++)
    {
        rand_nums[i] = (rand() / (float)RAND_MAX);

        // rand_nums[i] = ((float)rand()/(float)(RAND_MAX)) * num_elements;//create ran number with limit number of element
    }
    return rand_nums;
}

// Distance**2 between d-vectors pointed to by v1, v2.
float distanceBetween(const float *v1, const float *v2, const int d)
{
    float dist = 0.0;
    for (int i = 0; i < d; i++)
    {
        float diff = v1[i] - v2[i];
        dist += diff * diff;
    }
    return dist;
}

// Assign a seed to the correct cluster by computing its distances to
// each cluster centroid.
int assignSeed(const float *site, float *centroids,
                const int k, const int d)
{
    int best_cluster = 0;
    float best_dist = distanceBetween(site, centroids, d);
    float *centroid = centroids + d;
    for (int c = 1; c < k; c++, centroid += d)
    {
        float dist = distanceBetween(site, centroid, d);
        if (dist < best_dist)
        {
            best_cluster = c;
            best_dist = dist;
        }
    }
    return best_cluster;
}

// Add a seed (vector) into a sum of seeds (vector).
void addSeed(const float *site, float *sum, const int d)
{
    for (int i = 0; i < d; i++)
    {
        sum[i] += site[i];
    }
}

// Print the centroids one per line.
void notifyChangeCentroids(float *centroids, const int k, const int d)
{
    FILE *fpo= fopen("./data/old_centroids.dat","a");

    float *p = centroids;
    printf("Centroids:\n");
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < d; j++, p++)
        {
            printf("%f ", *p);
             fprintf(fpo, "%f\t", *p);
        }
        fprintf(fpo, "\n");
        printf("\n");
    }
    fclose(fpo);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr,
                "Usage:mpirun -n <number process> <compile file> <k-mean number> <number dimensions> <number seeds> \n");
        exit(1);
    }

    // Get stuff from command line:
    // number of seeds per processor.
    // number of processors comes from mpirun command line.  -n
    int k = atoi(argv[1]);         // number of clusters.
    int dimension = atoi(argv[2]); // dimension of data.
    int numberOfSeed = atoi(argv[3]);
    double start, end; //time start and end
    // Seed the random number generator to get different results each time
     srand(time(NULL));
    // srand(9999);

   

    // Initial MPI and find process rank and number of processes.
    MPI_Init(NULL, NULL);
    int rank, sizeRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &sizeRank);
    MPI_Barrier(MPI_COMM_WORLD);
    
    start = MPI_Wtime();
    //
    // Data structures in all processes.
    //
    // The seeds assigned to this process.
    float *seeds;
    assert(seeds = malloc(numberOfSeed * dimension * sizeof(float)));
    // The sum of seeds assigned to each cluster by this process.
    // k vectors of d elements.
    float *sums;
    assert(sums = malloc(k * dimension * sizeof(float)));
    // The number of seeds assigned to each cluster by this process. k integers.
    int *counts;
    assert(counts = malloc(k * sizeof(int)));
    // The current centroids against which seeds are being compared.
    // These are shipped to the process by the root process.
    float *centroids;
    assert(centroids = malloc(k * dimension * sizeof(float)));
    // The cluster assignments for each site.
    int *labels;
    assert(labels = malloc(numberOfSeed * sizeof(int)));

    //
    // Data structures maintained only in root process.
    //
    // All the seeds for all the processes.
    // site_per_proc * nprocs vectors of d floats.
    float *allSeeds = NULL;
    // Sum of seeds assigned to each cluster by all processes.
    float *seedSums = NULL;
    // Number of seeds assigned to each cluster by all processes.
    int *seedCounts = NULL;
    // Result of program: a cluster label for each seed.
    int *all_labels;
    if (rank == 0)
    {
        allSeeds = createRandomNums(dimension * numberOfSeed * sizeRank);
        // Take the first k seeds as the initial cluster centroids.
        for (int i = 0; i < k * dimension; i++)
        {
            centroids[i] = allSeeds[i];
        }
        notifyChangeCentroids(centroids, k, dimension);
        assert(seedSums = malloc(k * dimension * sizeof(float)));
        assert(seedCounts = malloc(k * sizeof(int)));
        assert(all_labels = malloc(sizeRank * numberOfSeed * sizeof(int)));
    }

    // Root sends each process its share of seeds.
    MPI_Scatter(allSeeds, dimension * numberOfSeed, MPI_FLOAT, seeds,
                dimension * numberOfSeed, MPI_FLOAT, 0, MPI_COMM_WORLD);

    float distance = 1.0; // Will tell us if centroids have moved.

    while (distance > 0.00001)
    { // While they've moved...

        // Broadcast the current cluster centroids to all processes.
        MPI_Bcast(centroids, k * dimension, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Each process reinitializes its cluster accumulators.
        for (int i = 0; i < k * dimension; i++)
            sums[i] = 0.0;
        for (int i = 0; i < k; i++)
            counts[i] = 0;

        // Find the closest centroid to each site and assign to cluster.
        float *site = seeds;
        for (int i = 0; i < numberOfSeed; i++, site += dimension)
        {
            int cluster = assignSeed(site, centroids, k, dimension);
            // Record the assignment of the site to the cluster.
            counts[cluster]++;
            addSeed(site, &sums[cluster * dimension], dimension);
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
            printf("Better distance: %f\n", distance);
            // Copy new centroids from grand_sums into centroids.
            for (int i = 0; i < k * dimension; i++)
            {
                centroids[i] = seedSums[i];
            }
            notifyChangeCentroids(centroids, k, dimension);
        }
        // Broadcast the norm.  All processes will use this in the loop test.
        MPI_Bcast(&distance, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    // Now centroids are fixed, so compute a final label for each site.
    float *site = seeds;
    for (int i = 0; i < numberOfSeed; i++, site += dimension)
    {
        labels[i] = assignSeed(site, centroids, k, dimension);
    }

    // Gather all labels into root process.
    MPI_Gather(labels, numberOfSeed, MPI_INT,
               all_labels, numberOfSeed, MPI_INT, 0, MPI_COMM_WORLD);

    // Root can print out all seeds and labels.
    if ((rank == 0) && 1)
    {
        printf("\nFinal clustering result with centroid tag:\nx\ty\ttag\n");
        FILE *fp = fopen("./data/output.dat", "w");
        FILE *fp1 = fopen("./data/output1.dat", "w");
        FILE *fp2 = fopen("./data/output2.dat", "w");
        FILE *fpc = fopen("./data/centroids.dat", "w");
        for (size_t i = 0; i < k; i++)
        {
            for (size_t j = 0; j < dimension; j++, centroids++)
            {
                fprintf(fpc, "%f\t", *centroids);
            }
            fprintf(fpc, "\n");
        }

        float *site = allSeeds;
        for (int i = 0; i < sizeRank * numberOfSeed; i++, site += dimension)
        {

            for (int j = 0; j < dimension; j++)
            {
                printf("%f ", site[j]);

                if (all_labels[i] == 0)
                {
                    fprintf(fp, "%f\t", site[j]);
                }
                else if(all_labels[i] == 1)
                {
                    fprintf(fp1, "%f\t", site[j]);
                }
                else
                {
                    fprintf(fp2, "%f\t", site[j]);
                }
            }
            if (all_labels[i] == 0)
            {
                fprintf(fp, "%4d\n", all_labels[i]);
            }
            else if(all_labels[i] == 1)
            {
                fprintf(fp1, "%4d\n", all_labels[i]);
            }
            else
            {
                fprintf(fp2, "%4d\n", all_labels[i]);
            }
            

            printf("%4d\n", all_labels[i]);
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
