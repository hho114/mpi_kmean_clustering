# cpsc479-project2

## MPI K-means Clustering

Group members:

Huy Ho hho114@csu.fullerton.edu

Chandler Ebrahimi csebra@csu.fullerton.edu

Darren Vu vuchampion@csu.fullerton.edu

## Requires library or packages

- [gnuplot](http://www.gnuplot.info/download.html)

- mpi or mpich

Note: This program was tested and compiled on Linux operation system

## Implement MPI

Consider N data points each of it is vector and P processors.

Step 1: Assign N/P data points to each processor.

Step 2: Node 0 randomly choose K points and assigns them as cluster means and broadcast.

Step 3: In each processor for each data point find membership using the cluster mean.

Step 4: Recalculate local means for each cluster in each processor.

Step 5: Globally broadcast all local means for each processor find the global mean.

Step 6: Go to step (3) and repeat until the number of iterations > 10000 or number of points where membership  has changed is less than 0.1 %.

## How to use

Go to this project mpi_kmean_clustering directory, and use mpicc to compile the main.c file (mpicc main.c), then run the compile file with four input arguements.(mpirun -n "number process" a.out "k number or number of cluster" "number dimension" "total data point"

For example:

```terminal

 mpicc main.c && mpirun -n 6 a.out 3 2 50

```

The program will print process of kmean algorithm change centroids and print the final result which show the point x y coordinates and centroid tag number.

For better understanding how k-mean work, we develop program to ouput image files which show how the centroids changes. However this will only work for 2 dimension and k number limit to 3.

## 2 Kmean Clustering

Use this command to create kmean cluster with 2 clusters and 100 data points per cluster graph:
Standard: mpicc main.c && mpirun -n "number process" a.out 2 2 "number data points"

```terminal

 mpicc main.c && mpirun -n 10 a.out 2 2 100 && gnuplot graphs/2_kmean_graph.gp

```

The output will look like this:

Changing centroids Process

![](images/2k_moving_centroids.png)

Final centroid when finish

![](images/2k_final_centroids.png)

## 3 Kmean Clustering

Use this command to create kmean cluster with 3 clusters and 100 data points per cluster graph:

Standard: mpicc main.c && mpirun -n "number process" a.out 3 2 "number data points"

```terminal

  mpicc main.c && mpirun -n 10 a.out 3 2 100 && gnuplot graphs/3_kmean_graph.gp

```

The output will look like this:

Changing centroids Process

![](images/3k_moving_centroids.png)

Final centroid when finish

![](images/3k_final_centroids.png)

### Shortcut command to run both method above

```terminal

make

```
