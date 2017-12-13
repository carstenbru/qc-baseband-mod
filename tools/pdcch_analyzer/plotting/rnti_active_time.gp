#!/usr/bin/gnuplot

# Plots the RNTI active time distribution. Each line of the file is considered as a new dataset. Use the TimeAverageWriter of dump_analyze to average over the desired time interval.

# examples:
#   gnuplot -c ./rnti_active_time.gp 10min_avg_pdcch_0.csv rnti_active_time.svg
#   gnuplot -c ./rnti_active_time.gp 10min_avg_pdcch_0.csv rnti_active_time.png

normalize = 0 #1 to normalize values (i.e. get the probability) or 0 to plot absolute values
width = 1280
height = 720
inp_file = ARG1
out_file = ARG2

rnti_active_time_start_column = 0
rnti_active_time_classes = 0

column = 0
ctext = "not empty"
while (ctext ne "") { # detect columns and other input data format parameters (e.g. number of classes)
    ctext = system("head -2 " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $".(column+1)."}' | tail -1")
    if (ctext[0:18] eq "RNTIs got inactive") {
        rnti_got_inactive_column = column
    }
    if (ctext[0:11] eq "active time") { 
        rnti_active_time_classes = rnti_active_time_classes+1 #number of classes for RNTI active time
        if (rnti_active_time_start_column == 0) {
            rnti_active_time_start_column = column #first colum with RNTI active time data
        }
    }
    column = column+1
}

if (out_file[strstrt(out_file,".")+1:] eq "png") set terminal png size width, height
if (out_file[strstrt(out_file,".")+1:] eq "svg") set terminal svg size width, height
if (out_file[strstrt(out_file,".")+1:] eq "eps") set terminal postscript eps size width/140, height/140 color
if (out_file[strstrt(out_file,".")+1:] eq "") set terminal qt size width, height; \
    else set output out_file

set datafile separator "\t"

set xlabel "RNTI active time"
if (normalize == 0) {
    set ylabel "Observations"
} else {
    set ylabel "Relative observations"
}
set xtics rotate

set tics front

n_lines = int(system("wc -l ". inp_file)) - 2

set style fill solid border -1
shift = 0.8/n_lines
set boxwidth 0.7/n_lines

array sums[n_lines]
do for [tp=1:n_lines] {
    if (normalize == 1) {
        sums[tp] = int(system("head -". (tp+2) ." " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $".(rnti_got_inactive_column+1)."}' | tail -1"));
    } else {
        sums[tp] = 1;
    }
}

plot for [tp=1:n_lines] inp_file matrix using ($1-rnti_active_time_start_column+tp*shift):($3/sums[tp]):xticlabel((tp == ceil(n_lines/2.0)) ? system("head -2 " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $".(int($1) + 1)."}' | tail -1")[13:] : "") every ::rnti_active_time_start_column:tp:rnti_active_time_start_column+rnti_active_time_classes-1:tp title system("head -". (tp+2) ." " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $1}' | tail -1") with boxes
