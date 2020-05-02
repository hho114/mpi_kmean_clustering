all: run

run: main
	mpirun -np 5 a.out 2 2 1000 && gnuplot graphs/2_kmean_graph.gp
	mpirun -np 5 a.out 3 2 1000 && gnuplot graphs/3_kmean_graph.gp

headers: functions.h

main: headers main.c
	mpicc main.c

clean:
	rm *.o