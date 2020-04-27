

// Distance**2 between d-vectors pointed to by v1, v2.
float distanceBetween(const float *point1, const float *point2, const int dimension)
{
    float dist = 0.0;
    for (int i = 0; i < dimension; i++)
    {
        float diff = point1[i] - point2[i];
        dist += diff * diff;
    }
    return dist;
}

// Assign a seed to the correct cluster by computing its distances to
// each cluster centroid.
int assignSeed(const float *seed, float *centroids,
               const int k, const int dimension)
{
    int bestCluster = 0;
    float bestDist = distanceBetween(seed, centroids, dimension);
    float *centroid = centroids + dimension;
    for (int c = 1; c < k; c++, centroid += dimension)
    {
        float dist = distanceBetween(seed, centroid, dimension);
        if (dist < bestDist)
        {
            bestCluster = c;
            bestDist = dist;
        }
    }
    return bestCluster;
}

// Add a seed (vector) into a sum of seeds (vector).
void addSeed(const float *seed, float *sum, const int dimension)
{
    for (int i = 0; i < dimension; i++)
    {
        sum[i] += seed[i];
    }
}

// Print the centroids one per line.
void notifyChangeCentroids(float *centroids, const int k, const int dimension, int *counter)
{
    FILE *fpo;
   if(*counter == 1)
   {
       fpo = fopen("./data/old_centroids.dat", "w");
   }
   else
   {
       
      fpo = fopen("./data/old_centroids.dat", "a");
   }
   
   
    float *p = centroids;
    printf("New centroids:\n");
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < dimension; j++, p++)
        {
            printf("%f ", *p);
            fprintf(fpo, "%f\t", *p);
        }
        fprintf(fpo, "\n");
        printf("\n");
    }
    
    fclose(fpo);

    *counter = *counter+1;
    
   
}

void inittialCentroids(float *centroids, const int k, const int dimension)
{
    FILE *fpo = fopen("./data/init_centroids.dat", "w");

    float *p = centroids;
    printf("Initital centroids:\n");
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < dimension; j++, p++)
        {
            printf("%f ", *p);
            fprintf(fpo, "%f\t", *p);
        }
        // fprintf(fpo, "\n");
        printf("\n");
    }

    fclose(fpo);
}

// Creates an array of random floats. Each number has a value from 0 - 1
float *createRandomNums(const int totalSeeds)
{
    float *rand_nums = (float *)malloc(sizeof(float) * totalSeeds);
    rand_nums != NULL;
    for (int i = 0; i < totalSeeds; i++)
    {
        rand_nums[i] = (rand() / (float)RAND_MAX);

        // rand_nums[i] = ((float)rand()/(float)(RAND_MAX)) * num_elements;//create ran number with limit number of element
    }
    return rand_nums;
}
