
set title 'K-mean Clustering'
set xlabel 'x'
set ylabel 'y'

set grid
set key below center horizontal noreverse enhanced autotitle box dashtype solid
set tics out nomirror
set border 3 front linetype black linewidth 1.0 dashtype solid

# set xrange [0.9:5.2]
# set xrange [0:5]
# set xtics 0, 1, 5
# set mxtics 1

# set yrange [0:5]

set style line 1 linecolor rgb '#0060ad' linewidth 2 pointtype 2 ps 1
set style line 2 linecolor rgb '#ff0000' linewidth 2 pointtype 19 ps 1
set style line 3 linecolor rgb '#4764F1' linewidth 2 pointtype 3 ps 1
set style line 4 linecolor rgb 'green'   linewidth 2 pointtype 7 ps 1

set terminal png enhanced
set output './images/3k_final_centroids.png'
plot './data/output.dat' using 1:2 with points linestyle 1 title 'Cluster A', \
	'./data/output1.dat' using 1:2 with points linestyle 2 title 'Cluster B', \
    './data/output2.dat' using 1:2 with points linestyle 3 title 'Cluster C', \
    './data/centroids.dat' using 1:2 with points linestyle 4 title 'Centroids'


set terminal png enhanced
set output './images/3k_moving_centroids.png'
# plot 'output.dat' using 1:2 with points linestyle 1 title 'Data points in Clusters'
plot './data/output.dat' using 1:2 with points linestyle 1 title 'Cluster A', \
	'./data/output1.dat' using 1:2 with points linestyle 2 title 'Cluster B', \
    './data/output2.dat' using 1:2 with points linestyle 3 title 'Cluster C', \
    './data/old_centroids.dat' using 1:2 with points linestyle 4 title 'Centroids'

# set terminal xterm
# replot
