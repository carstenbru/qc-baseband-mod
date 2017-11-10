#!/bin/sh

#example: ./generate_all_plots.sh pdcch_0

gnuplot --persist -c ./it_sfn.gp it_sfn_$1.csv it_sfn_$1.png it_sfn_$1_rb.png
gnuplot --persist -c ./sfn.gp sfn_$1.csv sfn_$1.png sfn_$1_rb.png
gnuplot --persist -c ./sfn_single.gp sfn_$1.csv 4 sfn_$1_4.png sfn_$1_4_rb.png
