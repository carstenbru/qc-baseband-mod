#!/usr/bin/gnuplot

# Plots the uplink MCS distribution. Each line of the file is considered as a new dataset. Use the TimeAverageWriter of dump_analyze to average over the desired time interval.

# examples:
#   gnuplot -c ./mcs_ul.gp 10min_avg_pdcch_0.csv mcs_ul.svg
#   gnuplot -c ./mcs_ul.gp 10min_avg_pdcch_0.csv mcs_ul.png

start_column = 33 #first colum with UL MCS data, to adapt to different data formats (different analyzers used)
width = 1280
height = 720
inp_file = ARG1
out_file = ARG2

if (out_file[strstrt(out_file,".")+1:] eq "png") set terminal png size width, height
if (out_file[strstrt(out_file,".")+1:] eq "svg") set terminal svg size width, height
if (out_file[strstrt(out_file,".")+1:] eq "eps") set terminal postscript eps size width/140, height/140 color
if (out_file[strstrt(out_file,".")+1:] eq "") set terminal qt size width, height; \
    else set output out_file
    
set datafile separator "\t"

set xlabel "MCS"
set ylabel "Probability"
set xtics rotate

set xrange [0:31]
set xtics 1
set tics front

set style rect fc lt -1 fs solid 0.12 noborder
set obj rect from 0, graph 0 to 10.5, graph 1 fc rgbcolor "red"
set obj rect from 10.5, graph 0 to 20.5, graph 1 fc rgbcolor "yellow"
set obj rect from 20.5, graph 0 to 28.5, graph 1 fc rgbcolor "green"

set label 1 at 4, graph 0.6
set label 1 "QPSK"
set label 2 at 14, graph 0.6
set label 2 "16-QAM"
set label 3 at 23, graph 0.6
set label 3 "64-QAM / 16-QAM"
set label 4 at 29.5, graph 0.48 rotate left
set label 4 "re-transmissions"

n_lines = int(system("wc -l ". inp_file)) - 2

plot for [tp=1:n_lines] inp_file matrix using ($1-start_column):3 every ::start_column:tp:start_column+31:tp title system("head -". (tp+2) ." " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $1}' | tail -1") with linespoints
