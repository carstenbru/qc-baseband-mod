#!/usr/bin/gnuplot

# plots the number of active users over time

# It supports (at least) the output of FrameAverageWriter, SfnIterationAverageWriter and TimeAverageWriter, resulting in different output scales.
# The input type is detected automatically, column numbers might need to be adapted at the beginning of this script.
# Data series can be excluded from the plot by setting the corresponding column variable to -1.
# Also plots for a single iteration of the SFN are supported by passing an additional argument with the desired iteration number.

# examples:
#   gnuplot -c ./active_users.gp it_avg_pdcch_0_dec.csv active_users.svg
#   gnuplot -c ./active_users.gp it_avg_pdcch_0_dec.csv active_users.svg

width = 1280
height = 720
inp_file = ARG1
out_file = ARG2

column = 0
ctext = "not empty"
while (ctext ne "") { # detect columns and other input data format parameters (e.g. number of classes)
    ctext = system("head -2 " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $".(column+1)."}' | tail -1")
    if (ctext[0:12] eq "active RNTIs") {
        active_users_column = column
    }
    column = column+1
}
active_users_column = active_users_column + 1

if (out_file[strstrt(out_file,".")+1:] eq "png") set terminal png size width, height
if (out_file[strstrt(out_file,".")+1:] eq "svg") set terminal svg size width, height
if (out_file[strstrt(out_file,".")+1:] eq "eps") set terminal postscript eps size width/140, height/140 color
if (out_file[strstrt(out_file,".")+1:] eq "") set terminal qt size width, height; \
    else set output out_file

set datafile separator "\t"

set ylabel "active RNTIs"
set xtics rotate

if (system("head -2 " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $1}' | tail -1") eq "Timestamp") {
    set xlabel "time [HH:MM:SS]"
    set xdata time
    set timefmt "%d.%m.%Y %H:%M:%S"
    set format x "%H:%M:%S"
    set xtics 60
        
    plot inp_file using 1:active_users_column title columnheader(active_users_column) with linespoints
} else {
    set xlabel "SFN iteration (10.24s)"
    set xtics 10
        
    plot inp_file using ($1 + ($2 / 1024)):active_users_column title columnheader(active_users_column) with points
}
