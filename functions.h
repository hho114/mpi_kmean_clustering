#ifndef FUNCTION
#define FUNCTION
// Calculate distance between point and centroid using the euclidean distance
float distanceBetween(const float *point, const float *centroid, const int dimension)
{
    float dist = 0.0;
    for (int i = 0; i < dimension; i++)
    {
        float diff = point[i] - centroid[i];
        dist += diff * diff;
    }
    return dist;
}

// Assign label to the correct cluster by computing its distances to each cluster centroid.
int assignLabel(const float *points, float *centroids,
                const int k, const int dimension)
{
    int bestCluster = 0;
    float bestDist = distanceBetween(points, centroids, dimension); //calculate first point and first centroid distance
    float *centroid = centroids + dimension;                        //skip first centroid
    for (int c = 1; c < k; c++, centroid += dimension)
    {
        float dist = distanceBetween(points, centroid, dimension);
        if (dist < bestDist)
        {
            bestCluster = c;
            bestDist = dist;
        }
    }
    return bestCluster;
}

// Add points into a sum of points.
void addPoint(const float *points, float *sums, const int dimension)
{
    for (int i = 0; i < dimension; i++)
    {
        sums[i] += points[i];
    }
}

// Print the centroids one per line.
void notifyUpdateCentroids(float *centroids, const int k, const int dimension, int *counter)
{
    FILE *fpo;
    if (*counter == 1)
    {
        fpo = fopen("./data/old_centroids.dat", "w");
    }
    else
    {

        fpo = fopen("./data/old_centroids.dat", "a");
    }

    float *p = centroids;

    for (int i = 0; i < k; i++)
    {
        printf("Update centroids: ");
        for (int j = 0; j < dimension; j++, p++)
        {
            printf("%f ", *p);
            fprintf(fpo, "%f\t", *p);
        }
        fprintf(fpo, "\n");
        printf("\n");
    }

    fclose(fpo);

    *counter = *counter + 1;
}

void inittialCentroids(float *centroids, const int k, const int dimension)
{
    FILE *fpo = fopen("./data/init_centroids.dat", "w");

    float *p = centroids;

    for (int i = 0; i < k; i++)
    {
        printf("Initital centroids: ");
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
float *createRandomNums(const int totalPoints)
{
    float *rand_nums = (float *)malloc(sizeof(float) * totalPoints);
    rand_nums != NULL;
    for (int i = 0; i < totalPoints; i++)
    {
        rand_nums[i] = (rand() / (float)RAND_MAX);

    }
    return rand_nums;
}

#endif