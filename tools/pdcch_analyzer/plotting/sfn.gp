#!/usr/bin/gnuplot --persist

# examples:
# plot to file: gnuplot --persist -c ./sfn.gp sfn_pdcch_0.csv sfn_pdcch_0.png sfn_pdcch_0_rb.png
# plot screen : gnuplot --persist -c ./sfn.gp sfn_pdcch_0.csv

set terminal qt size 1920,1080

if (ARG2 ne "") set terminal png size 1920,1080
if (ARG2 ne "") set output ARG2


set datafile separator "\t"
set multiplot;
set size 1,0.5; 
set xlabel "SFN iteration (10.24s)"
set ylabel "data rate [kbit/s]"
set xtics rotate

set xtics 10

set origin 0.0,0.5;
plot ARG1 using ($1 + ($2 / 1024)):3 title columnheader(3) with points
#plot ARG1 using ($1 + ($2 / 1024)):3 title columnheader(3) with linespoints, \
#    '' using ($1 + ($2 / 1024)):4 title columnheader(4) with linespoints, \
#    '' using ($1 + ($2 / 1024)):5 title columnheader(5) with linespoints

set origin 0.0,0.0;
plot ARG1 using ($1 + ($2 / 1024)):4 title columnheader(4) with points, \
    '' using ($1 + ($2 / 1024)):5 title columnheader(5) with points

unset multiplot;
if (ARG3 eq "") exit gnuplot
set output ARG3
set multiplot;

set ylabel "allocated RBs"

set origin 0.0,0.5;
plot ARG1 using ($1 + ($2 / 1024)):6 title columnheader(6) with points

set origin 0.0,0.0;
plot ARG1 using ($1 + ($2 / 1024)):7 title columnheader(7) with points, \
    '' using ($1 + ($2 / 1024)):8 title columnheader(8) with points
    
unset multiplot;
