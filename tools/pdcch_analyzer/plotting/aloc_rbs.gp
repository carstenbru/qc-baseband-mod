#!/usr/bin/gnuplot

# plots the number of allocated RBs (downlink)

# It supports (at least) the output of FrameAverageWriter and SfnIterationAverageWriter, resulting in different output scales.
# The input type is detected automatically, column numbers might need to be adapted at the beginning of this script.
# Data series can be excluded from the plot by setting the corresponding column variable to -1.
# Also plots for a single iteration of the SFN are supported by passing an additional argument with the desired iteration number.

# examples:
#   all data:                  gnuplot -c ./aloc_rbs.gp frame_avg_pdcch_0.csv aloc_rbs.svg
#   only 3rd iteration of SFN: gnuplot -c ./aloc_rbs.gp frame_avg_pdcch_0.csv aloc_rbs.png 3

#to adapt to different data formats (different analyzers used), set to -1 if not wanted in plot
cell_rbs_column = 6
paging_rbs_column = 7
ue_rbs_column = 8

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

set ylabel "allocated RBs"
set xtics rotate

set xtics 10

if (ARG3 eq "") {
    if (system("head -2 " . inp_file . " | awk 'BEGIN {FS=\"\t\"}; {print $1}' | tail -1") eq "Timestamp") {
        set xlabel "time [HH:MM:SS]"
        set xdata time
        set timefmt "%d.%m.%Y %H:%M:%S"
        set format x "%H:%M:%S"
        set xtics 60
        
        plot inp_file using 1:((cell_rbs_column >= 0) ? column(cell_rbs_column) : 1/0) title columnheader(cell_rbs_column) with linespoints, \
            '' using 1:((paging_rbs_column >= 0) ? column(paging_rbs_column) : 1/0) title columnheader(paging_rbs_column) with linespoints, \
            '' using 1:((ue_rbs_column >= 0) ? column(ue_rbs_column) : 1/0) title columnheader(ue_rbs_column) with linespoints
    } else {
        set xlabel "SFN iteration (10.24s)"
        set xtics 10
    
        plot inp_file using ($1 + ($2 / 1024)):((cell_rbs_column >= 0) ? column(cell_rbs_column) : 1/0) title columnheader(cell_rbs_column) with points, \
            '' using ($1 + ($2 / 1024)):((paging_rbs_column >= 0) ? column(paging_rbs_column) : 1/0) title columnheader(paging_rbs_column) with points, \
            '' using ($1 + ($2 / 1024)):((ue_rbs_column >= 0) ? column(ue_rbs_column) : 1/0) title columnheader(ue_rbs_column) with points
    }
} else {
    set xlabel "SFN"
    set xtics 32
    
    plot inp_file using 2:(((cell_rbs_column >= 0) && ($1==ARG3)) ? column(cell_rbs_column) : 1/0) title columnheader(cell_rbs_column) with linespoints, \
        '' using 2:(((paging_rbs_column >= 0) && ($1==ARG3)) ? column(paging_rbs_column) : 1/0) title columnheader(paging_rbs_column) with linespoints, \
        '' using 2:(((ue_rbs_column >= 0) && ($1==ARG3)) ? column(ue_rbs_column) : 1/0) title columnheader(ue_rbs_column) with linespoints
}
