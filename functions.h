

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
    int bestCluster = 0;
    float bestDist = distanceBetween(site, centroids, d);
    float *centroid = centroids + d;
    for (int c = 1; c < k; c++, centroid += d)
    {
        float dist = distanceBetween(site, centroid, d);
        if (dist < bestDist)
        {
            bestCluster = c;
            bestDist = dist;
        }
    }
    return bestCluster;
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
void notifyChangeCentroids(float *centroids, const int k, const int d, int *counter)
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
        for (int j = 0; j < d; j++, p++)
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

void inittialCentroids(float *centroids, const int k, const int d)
{
    FILE *fpo = fopen("./data/init_centroids.dat", "w");

    float *p = centroids;
    printf("Initital centroids:\n");
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < d; j++, p++)
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
    assert(rand_nums != NULL);
    for (int i = 0; i < totalSeeds; i++)
    {
        rand_nums[i] = (rand() / (float)RAND_MAX);

        // rand_nums[i] = ((float)rand()/(float)(RAND_MAX)) * num_elements;//create ran number with limit number of element
    }
    return rand_nums;
}
