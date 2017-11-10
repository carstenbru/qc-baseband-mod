#!/usr/bin/gnuplot --persist

# examples:
# plot to file: gnuplot --persist -c ./it_sfn.gp it_sfn_pdcch_0.csv it_sfn_pdcch_0.png it_sfn_pdcch_0_rb.png
# plot screen : gnuplot --persist -c ./it_sfn.gp it_sfn_pdcch_0.csv

set terminal qt size 1920,1080

if (ARG2 ne "") set terminal png size 1920,1080
if (ARG2 ne "") set output ARG2


set datafile separator "\t"
set multiplot;
set size 1,0.5; 
set xlabel "time [HH:MM:SS]"
set ylabel "data rate [kbit/s]"
set xtics rotate

set xdata time
set timefmt "%d.%m.%Y %H:%M:%S"
set format x "%H:%M:%S"
set xtics 60

set origin 0.0,0.5;
plot ARG1 using 1:4 title columnheader(4) with linespoints, \
    '' using 1:5 title columnheader(5) with linespoints, \
    '' using 1:3 title columnheader(3) with linespoints

set origin 0.0,0.0;
plot ARG1 using 1:4 title columnheader(4) with linespoints, \
    '' using 1:5 title columnheader(5) with linespoints

unset multiplot;
if (ARG3 eq "") exit gnuplot
set output ARG3
set multiplot;

set ylabel "allocated RBs"

set origin 0.0,0.5;
plot ARG1 using 1:7 title columnheader(7) with linespoints, \
    '' using 1:8 title columnheader(8) with linespoints, \
    '' using 1:6 title columnheader(6) with linespoints

set origin 0.0,0.0;
plot ARG1 using 1:7 title columnheader(7) with linespoints, \
    '' using 1:8 title columnheader(8) with linespoints
    
unset multiplot;

