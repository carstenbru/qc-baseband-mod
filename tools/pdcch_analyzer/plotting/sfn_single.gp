#!/usr/bin/gnuplot --persist

# examples:
# plot to file: gnuplot --persist -c ./sfn_single.gp sfn_pdcch_0.csv 3 sfn_single_pdcch_0_3.png sfn_single_pdcch_0_3_rb.png
# plot screen : gnuplot --persist -c ./sfn_single.gp sfn_pdcch_0.csv 3

set terminal qt size 1920,1080

if (ARG3 ne "") set terminal png size 1920,1080
if (ARG3 ne "") set output ARG3


set datafile separator "\t"
set multiplot;
set size 1,0.5; 
set xlabel "SFN"
set ylabel "data rate [kbit/s]"
set xtics rotate
#set key left top

set xtics 32

set origin 0.0,0.5;
plot ARG1 using 2:($1==ARG2?$4:1/0) title columnheader(4) with linespoints, \
    '' using 2:($1==ARG2?$5:1/0) title columnheader(5) with linespoints, \
    '' using 2:($1==ARG2?$3:1/0) title columnheader(3) with linespoints

set origin 0.0,0.0;
plot ARG1 using 2:($1==ARG2?$4:1/0) title columnheader(4) with linespoints, \
    '' using 2:($1==ARG2?$5:1/0) title columnheader(5) with linespoints

unset multiplot;
if (ARG4 eq "") exit gnuplot
set output ARG4
set multiplot;

set ylabel "allocated RBs"

set origin 0.0,0.5;
plot ARG1 using 2:($1==ARG2?$7:1/0) title columnheader(7) with linespoints, \
    '' using 2:($1==ARG2?$8:1/0) title columnheader(8) with linespoints, \
    '' using 2:($1==ARG2?$6:1/0) title columnheader(6) with linespoints

set origin 0.0,0.0;
plot ARG1 using 2:($1==ARG2?$7:1/0) title columnheader(7) with linespoints, \
    '' using 2:($1==ARG2?$8:1/0) title columnheader(8) with linespoints
    
unset multiplot;
