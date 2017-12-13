#!/usr/bin/gnuplot

# Plots the RNTI data consumption distribution (downlink & uplink). Each line of the file is considered as a new dataset. Use the TimeAverageWriter of dump_analyze to average over the desired time interval.

# examples:
#   gnuplot -c ./rnti_data.gp 10min_avg_pdcch_0.csv rnti_data_dl.svg dl
#   gnuplot -c ./rnti_data.gp 10min_avg_pdcch_0.csv rnti_data_ul.png ul

normalize = 0 #1 to normalize values (i.e. get the probability) or 0 to plot absolute values
width = 1280
height = 720
inp_file = ARG1
out_file = ARG2
data_direction = ARG3

rnti_dl_data_start_column = 0
rnti_ul_data_start_column = 0
rnti_dl_data_classes = 0
rnti_ul_data_classes = 0

column = 0
ctext = "not empty"
while (ctext ne "") { # detect columns and other input data format parameters (e.g. number of classes)
    ctext = system("head -2 " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $".(column+1)."}' | tail -1")
    if (ctext[0:18] eq "RNTIs got inactive") {
        rnti_got_inactive_column = column
    }
    if (ctext[0:14] eq "DL transmitted") { 
        rnti_dl_data_classes = rnti_dl_data_classes+1 #number of classes for RNTI DL data
        if (rnti_dl_data_start_column == 0) {
            rnti_dl_data_start_column = column #first colum with RNTI dl data
        }
    }
    if (ctext[0:14] eq "UL transmitted") { 
        rnti_ul_data_classes = rnti_ul_data_classes+1 #number of classes for RNTI UL data
        if (rnti_ul_data_start_column == 0) {
            rnti_ul_data_start_column = column #first colum with RNTI ul data
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

if (data_direction eq "ul") {
    set xlabel "RNTI UL data consumption"
    start_column = rnti_ul_data_start_column
    num_classes = rnti_ul_data_classes
} else {
    set xlabel "RNTI DL data consumption"
    start_column = rnti_dl_data_start_column
    num_classes = rnti_dl_data_classes
}

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

plot for [tp=1:n_lines] inp_file matrix using ($1-start_column+tp*shift):($3/sums[tp]):xticlabel((tp == ceil(n_lines/2.0)) ? system("head -2 " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $".(int($1) + 1)."}' | tail -1")[16:] : "") every ::start_column:tp:start_column+num_classes-1:tp title system("head -". (tp+2) ." " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $1}' | tail -1") with boxes
