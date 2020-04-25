all: run

run: main
	mpirun -n 10 a.out 2 2 100 && gnuplot graphs/2_kmean_graph.gp
	mpirun -n 10 a.out 3 2 100 && gnuplot graphs/3_kmean_graph.gp

headers: functions.h

main: headers main.c
	mpicc main.c

clean:
	rm *.out