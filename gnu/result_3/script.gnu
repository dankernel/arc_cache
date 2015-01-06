
set terminal postscript enhanced mono
set term post font ",20"
set output "gnuplot.eps"

#Style
set style data linespoints

#Title
set title "LRU cache hit ratio hm_1"

#Key
set key bottom

#Lable
set ylabel "Hit rato(%)"
set xlabel "Cache size(2^n)"

#yrange
set yrange [0:100]

#Xtic rotate(Not do)
set xtic rotate by 0 scale 1

#Print (2 to 3)
plot for [i=2:3] 'result.dat' using i:xtic(1) title columnheader(i)
set output
