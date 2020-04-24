// /*  Not complete code in MPI for Distributed k-means clustering*/
// p = MPI_Comm_size();
// r = MPI_Comm_rank();
// previousMSE = 0;
// /* Mean Square Error, the cost function for the k-means */
// MSE =∞; // infinity
// if r = 0 then
//   /* Initialize randomly the cluster seeds */
//   Initialize C = (c1, . . . , ck );
//   MPI_Bcast(C, 0);
// end
// while MSE = previousMSE do
//   previousMSE = MSE;
//   MSE_t = 0;
//   for j = 1 to k do
//      m_t[j]= 0;
//      n_t[j] = 0;
//   end
//   for i = r (n/p) to (r + 1)(n/p) − 1 do
//      for j = 1 to k do
//         Calculate d[i][j] = d(x[i] ,m[j]) **2 ; // = ||x[i ]− m[j] ||^2 2;
//      end
//      // Find the closest centroid ml to xi : l = argmin j di, j ;
//      /* Update stage */
//      m_t[l] = m_t[l] + x[i ];
//      n_t[l] = n_t [l] + 1;
//      MSE_t = MSE_t + d(x[i] ,m[l])**2;
//   end
//   /* Aggregate: make use of the composability property of centroids */
//   for j = 1 to k do
//      MPI_Allreduce(n_t[j] , n[j] , MPI_SUM);
//      MPI_Allreduce(m_t[j] ,m[j] , MPI_SUM);
//      /* To prevent dividing by zero */
//      n[ j ]= max(n [j ], 1);
//      m[ j] = m [j] /n [j] ;
//   end
//   /* Update the cost function */
//   MPI_Allreduce(MSE_t,MSE, MPI_SUM);
// end


#include <stdio.h>
#include <mpi.h>
#include <limits.h>
#include<stdlib.h>




int main(int argc, char *argv[])   {    
     
    int rank;    
    int size; 
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);    
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    srand(time(NULL));
    int previousMSE = 0;
    int MSE = INT_MAX;
    int numberOfSeed = atoi(argv[1]);
    int numberDimension = atoi(argv[2]);
    
    
    float * seeds = malloc(numberOfSeed * numberDimension * sizeof(float)); // numberOfSeed is row, numberDimension is column

    if (rank == 0)
    {
        int  r;
       for (int i = 0; i <  numberOfSeed; i++) 
       {
        for (int j = 0; j < numberDimension; j++) 
        {
            r = ((float)rand()/(float)(RAND_MAX)) * 99;
           
            *(seeds + i * numberDimension + j) = r; 
        }
       }

        MPI_Bcast(seeds, numberOfSeed * numberDimension, MPI_FLOAT, 0, MPI_COMM_WORLD);
       
    }

    while (MSE == previousMSE)
    {
        previousMSE = MSE;
        int MSE_t =0;
        int m_t[numberOfSeed],n_t[numberOfSeed];
        for (size_t i = 0; i < numberOfSeed; i++)
        {
            m_t[i]=0;
            n_t[i]=0;
        }

        for (size_t i = rank * (numberOfSeed/size); i < (rank+1)*(numberOfSeed/size)-1; i++)
        {
            for (size_t j = 1; j < numberOfSeed; j++)
            {
                
            }
            
        }
        
        
    }
    
   
    

    
         
     MPI_Finalize();    
     return 0;
}
